
//author:1067720863@qq.com
//create date:2023/04
//decription: PBR Material

#include "PbrLightEffect.h"

#include <osgEarth/PhongLightingEffect>
#include <osgEarth/Registry>
#include <osgEarth/Capabilities>
#include <osgEarth/Shaders>
#include <osgEarth/Lighting>
#include <osgEarth/PhongLightingEffect>
#include <osgEarth/Math>
#include <osgEarth/FileUtils>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>

#include<osg/TexEnv>
#include<osg/Texture2DArray>
using namespace osgEarth;
using namespace osgEarth::Util;


//ShadersGL3 osgEarth::Util::PbrShadersFactory::s_gl3;
//ShadersGL4 osgEarth::Util::PbrShadersFactory::s_gl4;


const char* pbr_shader = R"(
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
    #pragma import_defines(cascade, OE_ENABLE_BASECOLOR_MAP,OE_ENABLE_NORMAL_MAP, OE_ENABLE_MR_MAP, OE_ENABLE_AO_MAP, OE_ENABLE_EMISSIVE_MAP)
   
    // #pragma vp_function BRDF, fragment, first
    // vp_function is a function which will be execute in specific stage, like fragment_color, can be added or removed dynamically 


    vec3 fresnel_GGX(float VdotH, vec3 f0)
    {
        return f0 + (1.0 - f0) * pow(1.0 - VdotH, 5);
    }

    vec3 fresnel_Epic(float VdotH, vec3 f0)
    {
        return f0 + (1 - f0) * exp2((-5.55473 * VdotH - 6.98316) * VdotH);
    }

    float microfacetDistribution(float NdotH, float roughness)
    {
        float roughness2 = roughness * roughness;
        float f = NdotH * NdotH * (roughness2 - 1) + 1.0;
        return roughness2 / (PI * f * f);
    }

    float geometricOcclusion(float NdotL, float NdotV, float roughness)
    {
        // End result of remapping:
        float k = pow(roughness + 1, 2) / 8.0f;
        float attenuationV = NdotV / (NdotV * (1 - k) + k);
        float attenuationL = NdotL / (NdotL * (1 - k) + k);
        return attenuationL * attenuationV;
    }

    vec3 BRDF_Diffuse_Lambert(vec3 albedo)
    {
        return albedo / PI;
    }

    vec3 BRDF_Diffuse_Disney(float VdotH, float NdotL, float NdotV,
        float roughness, vec3 diffuseColor)
    {
        float Fd90 = 0.5 + 2.0 * roughness * VdotH * VdotH;
        vec3 f0 = vec3(0.1f, 0.1f, 0.1f);
        vec3 invF0 = vec3(1.0, 1.0, 1.0) - f0;
        float dim = min(invF0.r, min(invF0.g, invF0.b));
        float result = ((1.0 + (Fd90 - 1.0) * pow(1.0 - NdotL, 5.0)) * (1.0 + (Fd90 - 1.0) * pow(1.0 - NdotV, 5.0))) * dim;
        return diffuseColor * result;
    }

    vec3 BRDF_Diffuse_Burley(float VdotH, float NdotL, float NdotV,
        float roughness, vec3 diffuseColor)
    {
        float energyBias = mix(roughness, 0.0, 0.5);            // 0.5
        float energyFactor = mix(roughness, 1.0, 1.0 / 1.51);   // 1/3.1415
        float fd90 = energyBias + 2.0 * VdotH * VdotH * roughness;
        float f0 = 1.0;
        float lightScatter = f0 + (fd90 - f0) * pow(1.0 - NdotL, 5.0);
        float viewScatter = f0 + (fd90 - f0) * pow(1.0 - NdotV, 5.0);
        float result = lightScatter * viewScatter * energyFactor;
        return diffuseColor * result;
    }

    vec3 CookTorranceSpecular(float VdotH, float NdotH, float NdotL, float NdotV,
        float roughness, float metalness, vec3 f0, out vec3 kS)
    {

        float D = microfacetDistribution(NdotH, roughness);
        vec3 F = fresnel_Epic(VdotH, f0);
        float G = geometricOcclusion(NdotL, NdotV, roughness);
        return (D * F * G) / (4 * max(NdotV * NdotL, 0.01f));
    }
    vec4 SRGBtoLINEAR(vec4 srgbIn)
    {
        vec3 linOut = pow(srgbIn.xyz, vec3(2.2f));
        return vec4(linOut, srgbIn.w);
    }

    vec4 LINEARtoSRGB(vec4 srgbIn)
    {
        vec3 linOut = pow(srgbIn.xyz, vec3(1.0 / 2.2f));
        return vec4(linOut, srgbIn.w);
    }

    vec3 BRDF(
             float VdotH,
             float NdotH,
             float NdotL,
             float NdotV,
             float roughness,
             float metalness,
             vec3 f0,
             vec3 diffuseColor,
             vec3 lightColor,
             float ao,
             vec3 emissive
            )
    {
        float D = microfacetDistribution(NdotH, roughness);
        vec3 F = fresnel_Epic(VdotH, f0);
        float G = geometricOcclusion(NdotL, NdotV, roughness);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metalness;


        vec3 numerator = D * G * F;
        float denominator = 4.0 * NdotV * NdotL;
        vec3 specContrib = numerator / max(denominator, 0.001);

        vec3 diffuse = BRDF_Diffuse_Burley(VdotH, NdotL, NdotV, roughness, diffuseColor);
        vec3 diffuseContrib = (vec3(1.0f, 1.0f, 1.0f) - F)  * diffuse;
    
        vec3 Lo = NdotL * lightColor * (diffuseContrib + specContrib);

        Lo *= ao;
        Lo += emissive;

        return Lo;
    
    }


    vec3 normalInWorldWithoutTangent(vec3 normal, vec3 eyePos,vec2 uv, vec3 normalFromTexture)
    {
        // Perturb normal, see http://www.thetenthplanet.de/archives/1180
        vec3 unpackedNormal = normalFromTexture * 2.0f - 1.0f;

        // vec3 q1 = ddx(eyePos);
        // vec3 q2 = ddy(eyePos);
        // vec2 st1 = ddx(uv);
        // vec2 st2 = ddy(uv);

        // vec3 N = normalize(normal);
        // vec3 T = normalize(q1 * st2.y - q2 * st1.y);
        // vec3 B = normalize(cross(N, T));

	    // mat3 TBN = mat3(T, B, N);
        // return normalize(TBN*unpackedNormal);
        return normal;

    }

    vec3 getNormal(vec3 normal, vec3 normalFromTexture)
    {

        vec3 unpackedNormal = normalFromTexture*2.0 -1.0;  // normal in tangent space
        vec3 tangent = vec3(1.0,0.0,0.0);

        vec3 N = normalize(normal);
        vec3 T =  normalize(tangent - N * dot(tangent, N));
        vec3 B = cross(N, T);
        mat3 TBN = mat3(T, B, N);
        return normalize(TBN * unpackedNormal);
	    //return normalize(mul(unpackedNormal, ));       // normal in world space

    }

    mat3 makeTBN(vec3 N, vec3 p, vec2 uv)
    {
        // get edge vectors of the pixel triangle
        vec3 dp1 = dFdx(p);
        vec3 dp2 = dFdy(p);
        vec2 duv1 = dFdx(uv);
        vec2 duv2 = dFdy(uv);

        // solve the linear system
        vec3 dp2perp = cross(dp2, N);
        vec3 dp1perp = cross(N, dp1);
        vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
        vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

        // construct a scale-invariant frame 
        float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
        return mat3(T * invmax, B * invmax, N);
    }


    in vec3 oe_posView;  // view space
    in vec2 oe_texcoord;
    in vec3 oe_normal;  // world space

    // stage global
    in vec3 vp_Normal;
    // in pbr_Material oe_pbr;


    // flat in oe_tex_lod_index
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

    uniform osg_LightSourceParameters osg_LightSource[OE_NUM_LIGHTS];
    uniform pbr_Material oe_pbr;
    #ifdef cascade
        uniform sampler2DArray pbrMaps;
    #endif

    #ifdef OE_USE_PBR
    void fragment_main_pbr(inout vec4 color)
    {
    #ifndef OE_LIGHTING
        return;
    #endif
        //pbr_material_input(color);
        vec3  baseColor = vec3(1.0);
        float lightIntensity = 5.0;
        vec3  f0 = vec3(0.04);

        vec3 normal = oe_normal;

        float roughnessFactor = oe_pbr.roughnessFactor;
        float metallicFactor = oe_pbr.metallicFactor;
        vec3 emissiveFactor = oe_pbr.emissiveFactor;

    
        float roughness = roughnessFactor;
        float metallic = metallicFactor;
        float ao = oe_pbr.aoStrength;
        vec3 emissive = emissiveFactor;
        vec3 diffuseColor =vec3(0.0);

        f0 = mix(f0, pow(baseColor,vec3(2.2)), vec3(oe_pbr.metallicFactor));
        diffuseColor = baseColor.rgb * (vec3(1.0) - f0);
        diffuseColor *= 1.0 - metallic;

        #ifdef cascade
            oe_texcoord = abs(oe_texcoord);
            #ifdef OE_ENABLE_BASECOLOR_MAP
                diffuseColor = texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_BASECOLOR_MAP)).rgb;
            #endif

            #ifdef OE_ENABLE_MR_MAP
                vec3 tmp =  texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_MR_MAP)).rgb;
                metallic = metallicFactor * tmp.x;
                roughness = roughnessFactor * tmp.y;
            #endif

            #ifdef OE_ENABLE_NORMAL_MAP

                vec3 tangent = vec3(1.0,0.0,0.0);
                vec3 tangentNormal = texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_NORMAL_MAP)).rgb;  // normal in tangent space
                normal = getNormal(normal, tangentNormal);

            #endif

            #ifdef OE_ENABLE_AO_MAP
                float aoFromMap = texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_AO_MAP)).r;  // normal in tangent space
                ao *= aoFromMap;
            #endif

                #ifdef OE_ENABLE_EMISSIVE_MAP
                vec3 emissiveFromMap = texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_EMISSIVE_MAP)).rgb;  // normal in tangent space
                emissive *= emissiveFromMap;
            #endif
        #endif

        vec3 n = normalize(normal);
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
        //color.rgb *= 2.2;
        color.rgb = pow(color.rgb, vec3(1.0/2.2));
    

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

)";

osgEarth::Util::PbrLightEffect::PbrLightEffect()
{
    init();
}

osgEarth::Util::PbrLightEffect::PbrLightEffect(osg::StateSet* stateset)
{
    init();
    attach(stateset);
}


void osgEarth::Util::PbrLightEffect::attach(osg::StateSet* stateset)
{

    if (stateset && _supported)
    {
        _statesets.push_back(stateset);

        // set light
        stateset->setDefine(OE_LIGHTING_DEFINE, "1", osg::StateAttribute::ON);
        stateset->setDefine("OE_NUM_LIGHTS", "1");
        stateset->setDefine("OE_USE_PBR", "1");
        stateset->setDefine("cascade", "1");


        ShaderPackage shaders;
        VirtualProgram* pbrVP = VirtualProgram::getOrCreate(stateset);
        
        bool localTest = true;
        if (localTest)
        {
            auto shaderPath = "..//..//OE_PBR//Shader";
            auto shaderPath2 = "..//OE_PBR//Shader";

            osg::ref_ptr<osgDB::Options> dbo = new osgDB::Options();


            if (osgDB::fileExists(osgEarth::getAbsolutePath(shaderPath)))
            {
                dbo->setDatabasePath(osgEarth::getAbsolutePath(shaderPath));
            }
            else {
                dbo->setDatabasePath(osgEarth::getAbsolutePath(shaderPath2));
            }


            shaders.load(pbrVP, "pbr.glsl", dbo.get());
        }
        else {
            ShaderLoader::load(pbrVP, pbr_shader);
        }
       
  


    }
}


void osgEarth::Util::PbrLightEffect::detach()
{
    if (_supported)
    {
        for (StateSetList::iterator it = _statesets.begin(); it != _statesets.end(); ++it)
        {
            osg::ref_ptr<osg::StateSet> stateset;
            if ((*it).lock(stateset))
            {
                detach(stateset.get());
                (*it) = 0L;
            }
        }

        _statesets.clear();
    }
}

void osgEarth::Util::PbrLightEffect::detach(osg::StateSet* stateset)
{
    if (stateset && _supported)
    {
        //if ( _lightingUniform.valid() )
        //    stateset->removeUniform( _lightingUniform.get() );

        stateset->removeDefine(OE_LIGHTING_DEFINE);

        VirtualProgram* vp = VirtualProgram::get(stateset);
        if (vp)
        {
            Shaders shaders;
            shaders.unload(vp, shaders.PhongLighting);
        }
    }
}

osgEarth::Util::PbrLightEffect::~PbrLightEffect()
{
    detach();
}

void osgEarth::Util::PbrLightEffect::init()
{
    _supported = Registry::capabilities().supportsGLSL();
    _supported = true;
}
