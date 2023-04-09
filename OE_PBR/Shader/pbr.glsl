#pragma vp_name       Phong Lighting Vertex Stage
#pragma vp_entryPoint vertex_main_pbr
#pragma vp_location   vertex_view

#pragma import_defines(OE_LIGHTING, OE_NUM_LIGHTS)

vec3 vp_Normal;  //same as gl_Normal

out vec3 oe_posView;
out vec2 oe_texcoord;
out vec3 oe_normal;

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
    
}

[break]



#pragma vp_name       Pbr Lighting Fragment Stage
#pragma vp_entryPoint fragment_main_pbr
#pragma vp_location   fragment_lighting

#define PI 3.14159265359f

#pragma import_defines(OE_LIGHTING, OE_USE_PBR)
#pragma import_defines(OE_NUM_LIGHTS)
#pragma import_defines(OE_ENABLE_BASECOLOR_MAP,OE_ENABLE_NORMAL_MAP, OE_ENABLE_MR_MAP, OE_ENABLE_AO_MAP, OE_ENABLE_EMISSIVE_MAP)
#pragma include BRDF.glsl


in vec3 oe_posView;  // view space
in vec2 oe_texcoord;
in vec3 oe_normal;  // world space

// stage global
in vec3 vp_Normal;


// flat in oe_tex_lod_index
// varying decorated with flat, the data will not change when pass from vs to ps, and not introplote ,every fragment share same data

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

uniform osg_LightSourceParameters osg_LightSource[OE_NUM_LIGHTS];
// uniform pbr_Material oe_pbr;
#ifdef OE_ENABLE_BASECOLOR_MAP
    uniform sampler2D oe_basecolor_map ;
#endif
#ifdef OE_ENABLE_NORMAL_MAP
    uniform sampler2D oe_normal_map ;
#endif
#ifdef OE_ENABLE_MR_MAP    //metal roughness
    uniform sampler2D oe_mr_map ;
#endif
#ifdef OE_ENABLE_AO_MAP
    uniform sampler2D oe_ao_map ;
#endif
#ifdef OE_ENABLE_EMISSIVE_MAP
    uniform sampler2D oe_emissive_map ;
#endif


#ifdef OE_USE_PBR
void fragment_main_pbr(inout vec4 color)
{
#ifndef OE_LIGHTING
    return;
#endif

    vec3  baseColor = color.rgb;
    float lightIntensity = 5.0;
    vec3  f0 = vec3(0.04);

    float roughnessFactor = oe_pbr.roughnessFactor;
    float metallicFactor = oe_pbr.metallicFactor;
    vec3 emissiveFactor = oe_pbr.emissiveFactor;

    
    float roughness = 0.0;
    float metallic = 0.0;
    float ao = oe_pbr.aoStrength;
    vec3 emissive = emissiveFactor;
    vec3 diffuseColor =vec3(0.0);
    
    

    f0 = mix(f0, baseColor, vec3(oe_pbr.metallicFactor));
    diffuseColor = baseColor.rgb * (vec3(1.0) - f0);
    diffuseColor *= 1.0 - metallic;

    
    #ifdef OE_ENABLE_BASECOLOR_MAP
        diffuseColor *= texture(oe_basecolor_map, oe_texcoord);
    #endif

    #ifdef OE_ENABLE_MR_MAP
        vec3 tmp = texture(oe_basecolor_map, oe_texcoord).rgb;
        metallic = metallicFactor * tmp.x;
        roughness = roughnessFactor * tmp.y;
    #endif

    #ifdef OE_ENABLE_NORMAL_MAP

        vec3 tangent = vec3(1.0,0.0,0.0);
        vec3 tangentNormal = texture(oe_normal_map, oe_texcoord).rgb;  // normal in tangent space
        normal = getNormal(oe_normal, tangent, tangentNormal);
    #endif

    #ifdef OE_ENABLE_AO_MAP
        float aoFromMap = texture(oe_ao_map, oe_texcoord).r;  // normal in tangent space
        ao *= aoFromMap;
    #endif

     #ifdef OE_ENABLE_EMISSIVE_MAP
        vec3 emissiveFromMap = texture(oe_emissive_map, oe_texcoord).rgb;  // normal in tangent space
        emissive *= emissiveFromMap;
    #endif

    vec3 n = normalize(oe_normal);
    vec3 v = normalize(-oe_posView);
    float NdotV = max(dot(n, v), 0.0f);

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < OE_NUM_LIGHTS; ++i)
    {
        
        // per-light radiance: view space?
        vec3 l = normalize(osg_LightSource[i].position.xyz - oe_posView);
        vec3 h = normalize(l + v);


        float NdotL = max(dot(n, l), 0.0f);
        float VdotH = max(dot(h, v), 0.0f);
        float NdotH = max(dot(n, h), 0.0f);
        float NdotV = max(dot(n, v), 0.0f);
        
        vec3 lightColor = vec3(osg_LightSource[i].diffuse);


        Lo +=  BRDF(VdotH,NdotH, NdotL,NdotV,roughness, metallic,f0, diffuseColor,lightColor,ao,emissive);
    }

    vec3 ambient = osg_LightSource[0].ambient.rgb * diffuseColor * oe_pbr.aoStrength;

    color.rgb = ambient + Lo;

    // tone map:
    color.rgb = color.rgb / (color.rgb + vec3(1.0));

    // boost:
    color.rgb *= 2.2;

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
    color = color * vec4(1.0,0.0,0.0,1.0);
}
#endif
