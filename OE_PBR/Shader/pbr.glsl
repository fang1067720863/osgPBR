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
#pragma include BRDF.glsl
#pragma include light_functions.glsl


in vec3 oe_posView;  // view space
in vec2 oe_texcoord;
in vec3 oe_normal;  // world space
in vec3 oe_pos;
// stage global
in vec3 vp_Normal;
// in pbr_Material oe_pbr;


// varying decorated with flat, the data will not change when pass from vs to ps, and not introplote ,every fragment share same data
struct pbr_Material
{
    vec4  baseColorFactor;
    vec3  emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    float alphaMask;
    float alphaMaskCutoff;
    float aoStrength;
};
struct osg_LightSourceParameters 
{   
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   vec4 position;
   vec3 spotDirection;
   float spotExponent;
   float spotCutoff;
   float spotCosCutoff;
   float constantAttenuation;
   float linearAttenuation;
   float quadraticAttenuation;

   bool enabled;
};

struct ReflectedLight
{
	vec3 indirectDiffuse;
	vec3 indirectSpecular;
	vec3 directDiffuse;
	vec3 directSpecular;
};
struct GeometricContext
{
	vec3 normal;
	vec3 viewDir;
};

uniform osg_LightSourceParameters osg_LightSource[OE_NUM_LIGHTS];

uniform pbr_Material oe_pbr;
#ifdef cascade
    uniform sampler2DArray pbrMaps;
#endif
#ifdef USE_ENV_MAP
    uniform float envLightIntensity;
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

    # MATERIAL_BODY
#ifdef debug_texture
    return;
#endif

    
    diffuseColor = SRGBtoLINEAR(vec4(diffuseColor,1.0)).xyz;
    f0 = mix(f0, diffuseColor, vec3(metallic));
    diffuseColor *= (1.0 - metallic);
    
    vec3 n = normalize(normal);
    vec3 v = normalize(-oe_posView);
    float NdotV = max(dot(n, v), 0.0f);
    vec3 r = reflect(-v, n); 

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < OE_NUM_LIGHTS; ++i)
    {
        vec3 l = normalize(-osg_LightSource[i].spotDirection.xyz);
        vec3 h = normalize(l + v);

        float NdotL = max(dot(n, l), 0.0f);
        float VdotH = max(dot(h, v), 0.0f);
        float NdotH = max(dot(n, h), 0.0f);
        float NdotV = max(dot(n, v), 0.0f);
        
        vec3 lightColor = vec3(osg_LightSource[i].diffuse);
        Lo +=  BRDF(VdotH,NdotH, NdotL,NdotV,roughness, metallic,f0, diffuseColor,lightColor,ao);
        Lo *= osg_LightSource[i].spotExponent;
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

    ReflectedLight reflectedLight;
    reflectedLight.indirectDiffuse = vec3(0.0);
    reflectedLight.indirectSpecular = vec3(0.0);

#ifdef USE_ENV_MAP
    radianceIBL = getIBLRadiance(n, roughness,v);

    GeometricContext geometry;
    geometry.viewDir = v;
    geometry.normal = n;

    pbr_Material material;
    material.baseColorFactor=vec4(diffuseColor,1.0);
    material.metallicFactor=metallic;
    material.roughnessFactor=roughness;
    material.aoStrength=ao;

    RE_IndirectDiffuse_Physical(irradiance, diffuseColor, reflectedLight);
    RE_IndirectSpecular_Physical(radianceIBL, irradianceIBL,f0,geometry, material, reflectedLight);

#endif

    color.rgb = Lo;
    #ifdef OE_ENABLE_EMISSIVE_MAP
        color.rgb += emissive;
    #endif
    #ifdef USE_ENV_MAP
        color.rgb += (reflectedLight.indirectSpecular* ao * envLightIntensity + reflectedLight.indirectDiffuse * envLightIntensity);
    #endif


   

    
     // tonemap:
    float exposure = 2.2f;
    color.rgb *= exposure;
    //color.rgb = color.rgb / (color.rgb + vec3(1.0));
    
    color = linearTosRGB(color);

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

    // color.rgb = reflectedLight.indirectSpecular;
    // return; 
    // float tmp = reflectedLight.indirectSpecular.x;
    // if(reflectedLight.indirectSpecular.x<0.3)
    // {
    //     reflectedLight.indirectSpecular = vec3(0.0);
    // }else if (reflectedLight.indirectSpecular.x<0.7)
    // {
    //     reflectedLight.indirectSpecular = vec3(0.5);
    // }else{
    //     reflectedLight.indirectSpecular = vec3(1.0);
    // }
    // if(tmp<0.95)
    // {
    //     color.rgb =reflectedLight.indirectSpecular;
    // }else{
    //     color.rgb = vec3(0.0);
    // }
    // return;

        //reflectedLight.indirectDiffuse * oe_pbr.aoStrength + 
    //+ reflectedLight.indirectDiffuse;
    //+ reflectedLight.indirectDiffuse + reflectedLight.indirectSpecular;
    //irradianceIBL * 1/PI * baseColor;
    //reflectedLight.indirectDiffuse;
    // reflectedLight.indirectDiffuse;
