#pragma vp_name       Phong Lighting Vertex Stage
#pragma vp_entryPoint vertex_main_pbr
#pragma vp_location   vertex_view

#pragma import_defines(OE_LIGHTING, OE_NUM_LIGHTS)

vec3 vp_Normal;  //same as gl_Normal

out vec3 oe_posView;
out vec2 oe_texcoord;
out vec3 oe_normal;
out vec3 oe_pos;
/*vs define attribute binding and how todefine
*/ 
void vertex_main_pbr(inout vec4 VertexVIEW)
{
#ifndef OE_LIGHTING
    return;
#endif

    oe_posView = VertexVIEW.xyz / VertexVIEW.w;
    oe_texcoord = gl_MultiTexCoord0.xy;
    oe_normal = vp_Normal;
    oe_pos = gl_Vertex.xyz;
}

[break]



#pragma vp_name       Pbr Lighting Fragment Stage
#pragma vp_entryPoint fragment_main_pbr
#pragma vp_location   fragment_lighting

#define PI 3.14159265359f
#define RECIPROCAL_PI 0.318309886142228
// #define debug_texture 1
#pragma import_defines(MATERIAL_DEFINES)
#pragma import_defines(OE_LIGHTING, OE_USE_PBR, USE_ENV_MAP, USE_ENV_CUBE_UV)
#pragma import_defines(OE_NUM_LIGHTS)
#pragma import_defines(cascade, OE_ENABLE_BASECOLOR_MAP,OE_ENABLE_NORMAL_MAP, OE_ENABLE_MR_MAP, OE_ENABLE_AO_MAP, OE_ENABLE_EMISSIVE_MAP)
#pragma import_defines(USE_SHEEN,USE_CLEARCOAT,USE_TRANSMISSION)
#pragma import_defines(USE_CLEARCOATMAP,USE_CLEARCOAT_ROUGHNESSMAP, USE_SHEENCOLORMAP, USE_SHEENROUGHNESSMAP)
#pragma include normal_functions.glsl
#pragma include BRDF.glsl
#pragma include light_functions.glsl
#pragma include transmission.glsl
#pragma include struct.glsl


in vec3 oe_posView;  // view space
in vec2 oe_texcoord;
in vec3 oe_normal;  // view space
in vec3 oe_pos;
// stage global
in vec3 vp_Normal;


// varying decorated with flat, the data will not change when pass from vs to ps, and not introplote ,every fragment share same data


uniform osg_LightSourceParameters osg_LightSource[OE_NUM_LIGHTS];

uniform pbr_Material oe_pbr;
#ifdef cascade
    uniform sampler2DArray pbrMaps;
#endif
#ifdef USE_ENV_MAP
    uniform float envLightIntensity;
#endif

#ifdef USE_TRANSMISSION
    uniform sampler2D translucentMap;
#endif

# MATERIAL_UNIFORMS


vec4 linearTosRGB(in vec4 value)
{
    return vec4(mix(((pow(value.xyz, vec3(0.41666001, 0.41666001, 0.41666001)) * 1.0549999) - vec3(0.055, 0.055, 0.055)), \
    (value.xyz * 12.92), vec3(lessThanEqual(value.xyz, vec3(0.0031308001, 0.0031308001, 0.0031308001)))), value.w);
}
#ifdef OE_USE_PBR
void fragment_main_pbr(inout vec4 color)
{
#ifndef OE_LIGHTING
    return;
#endif
    vec3  baseColor = oe_pbr.baseColorFactor.rgb;
    float lightIntensity = 5.0;
    vec3  f0 = vec3(0.04);

    vec3 normal = oe_normal;
    vec3 normalEC = normal;

    float roughness = oe_pbr.roughnessFactor;
    float metallic = oe_pbr.metallicFactor;
    float ao = oe_pbr.aoStrength;
    vec3 emissive = oe_pbr.emissiveFactor;
    vec3 diffuseColor =vec3(1.0);

    pbr_Material material;
    GeometricContext geometry;
    ReflectedLight reflectedLight;
    
    # MATERIAL_BODY
#ifdef debug_texture
    return;
#endif

    f0 = mix(f0, diffuseColor, vec3(metallic));
    diffuseColor *= (1.0 - metallic);

    material.baseColorFactor=vec4(diffuseColor,1.0);
    material.metallicFactor=metallic;
    material.roughnessFactor=max(roughness, 0.04f);
    material.aoStrength=ao;
    material.specularF0 = f0;

    geometry.normal = normalize(normal);

    vec3 n = normalize(geometry.normal);
    vec3 v = normalize(-oe_posView);
    float NdotV = max(dot(n, v), 0.0f);
    vec3 r = reflect(-v, n); 

    vec3 Lo = vec3(0.0);

    
    geometry.viewDir = v;
    geometry.normal = n;

    
    reflectedLight.indirectDiffuse = vec3(0.0);
    reflectedLight.indirectSpecular = vec3(0.0);
    reflectedLight.directDiffuse = vec3(0.0);
    reflectedLight.directSpecular = vec3(0.0);


    for (int i = 0; i < OE_NUM_LIGHTS; ++i)
    {
        RE_Direct_Physical( osg_LightSource[i], geometry, f0, material, reflectedLight);
    }

    // indirect light accumulate
    vec3 ambient =vec3(0.0);
    vec3 irradiance = vec3(0.0);
    vec3 radiance = vec3(0.0);

    irradiance += osg_LightSource[0].ambient.rgb;
    // irradiance += getLightProbeIrradiance(); 
    // irradiance += getLightMapIrradiance();
    vec3 irradianceIBL = vec3(0.0);
    vec3 radianceIBL = vec3(0.0);

    
    vec3 clearcoatRadiance = vec3(0.0);
#ifdef USE_ENV_MAP
    radianceIBL = getIBLRadiance(n, roughness,v);
    irradianceIBL = getIBLIrradiance(n);
    RE_IndirectDiffuse_Physical(irradiance, diffuseColor, reflectedLight);

    #ifdef USE_CLEARCOAT
      clearcoatRadiance = getIBLRadiance(geometry.clearcoatNormal, material.clearcoatRoughness, v);
    #endif
     RE_IndirectSpecular_Physical(radianceIBL, irradianceIBL,clearcoatRadiance,f0,geometry, material, reflectedLight);

#endif

    vec3 totalDiffuse = reflectedLight.directDiffuse;
    vec3 totalSpecular = reflectedLight.directSpecular;

    
    #ifdef USE_ENV_MAP
        totalDiffuse += reflectedLight.indirectDiffuse * envLightIntensity;
        totalSpecular += reflectedLight.indirectSpecular* ao * envLightIntensity;
           
       
    #endif
    #ifdef USE_TRANSMISSION
        //SRGBtoLINEAR
        totalDiffuse = mix( totalDiffuse, vec4(reflectedLight.backLight,1.0f).rgb, material.transmission );
         float exposure = 2.2f;
        totalSpecular.rgb *= exposure;
        //color.rgb = color.rgb / (color.rgb + vec3(1.0));
        
        totalSpecular = linearTosRGB(vec4(totalSpecular,1.0f)).rgb;
        color.rgb = (totalDiffuse + totalSpecular);
    #else
        color.rgb = (totalDiffuse + totalSpecular);
    #endif
    #ifdef OE_ENABLE_EMISSIVE_MAP
        color.rgb += emissive;
    #endif
     

    #ifdef USE_SHEEN

		// Sheen energy compensation approximation calculation can be found at the end of
		// https://drive.google.com/file/d/1T0D1VSyR4AllqIJTQAraEIzjlb5h4FKH/view?usp=sharing
		float sheenEnergyComp = 1.0 - 0.157 * material.sheenColor.r;

		color.rgb = color.rgb * sheenEnergyComp + reflectedLight.sheenSpecular;
	#endif

    #ifdef USE_CLEARCOAT

		float dotNVcc = saturate( dot( geometry.clearcoatNormal, geometry.viewDir ) );

		vec3 Fcc = F_Schlick( material.clearcoatF0, material.clearcoatF90, dotNVcc );

		color.rgb =  color.rgb * ( 1.0 - material.clearcoat * Fcc ) + reflectedLight.clearcoatSpecular;

	#endif

   

    
     // tonemap:
   
    #ifdef USE_TRANSMISSION
       
        color.a = 0.1 + 0.1;
    #else
        float exposure = 2.2f;
        color.rgb *= exposure;
        //color.rgb = color.rgb / (color.rgb + vec3(1.0));
        
        color = linearTosRGB(color);
    #endif

    color.a = oe_pbr.alphaMask;


    // add in the haze
    //color.rgb += atmos_color;

    // exposure:
    //color.rgb = 1.0 - exp(-oe_sky_exposure * color.rgb);
    //color.rgb = 1.0 - exp(-oe_sky_exposure * 0.33 * color.rgb);

    // brightness and contrast
    //color.rgb = ((color.rgb - 0.5)*oe_pbr.contrast + 0.5) * oe_pbr.brightness;
}
#else
void fragment_main_pbr(inout vec4 color)
{
    // debug
    //color = color * vec4(1.0,0.0,0.0,1.0);
    color = texture(pbrMaps, vec3(oe_texcoord,4));
}
#endif