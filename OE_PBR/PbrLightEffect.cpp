#include "PbrLightEffect.h"

#include <osgEarth/PhongLightingEffect>
#include <osgEarth/Registry>
#include <osgEarth/Capabilities>
#include <osgEarth/Shaders>
#include <osgEarth/Lighting>
#include<osgEarth/PhongLightingEffect>
#include<osgDB/FileUtils>
#include <osgEarth/FileUtils>
#include<osgEarth/Math>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include<osg/TexEnv>
#include<osg/Texture2DArray>
using namespace osgEarth;
using namespace osgEarth::Util;


ShadersGL3 osgEarth::Util::PbrShadersFactory::s_gl3;
ShadersGL4 osgEarth::Util::PbrShadersFactory::s_gl4;

osg::Texture* createTextureAtlas()
{
    // Creates a texture array containing all the billboard images.
    // Each image is included only once.
    osg::Texture2DArray* tex = new osg::Texture2DArray();

    int arrayIndex = 0;
    float s = -1.0f, t = -1.0f;
    osg::ref_ptr<osg::Image> image_0 = osgDB::readRefImageFile("D:/GitProject/FEngine/Assets/PbrBox/BoomBox_normal.png");
    osg::ref_ptr<osg::Image> image_1 = osgDB::readRefImageFile("D:/GitProject/FEngine/Assets/PbrBox/BoomBox_roughnessMetallic.png");
    osg::ref_ptr<osg::Image> image_2 = osgDB::readRefImageFile("D:/GitProject/FEngine/Assets/PbrBox/BoomBox_occlusion.png");
    osg::ref_ptr<osg::Image> image_3 = osgDB::readRefImageFile("D:/GitProject/FEngine/Assets/PbrBox/BoomBox_emissive.png");
    osg::ref_ptr<osg::Image> image_4 = osgDB::readRefImageFile("D:/GitProject/FEngine/Assets/PbrBox/BoomBox_baseColor.png");

    s = osgEarth::nextPowerOf2(image_0->s());
    t = osgEarth::nextPowerOf2(image_0->t());
    std::cout << s << "" << t;
    tex->setTextureSize(s, t, 5);
    tex->setImage(0, image_0.get());
    tex->setImage(1, image_1.get());
    tex->setImage(2, image_2.get());
    tex->setImage(3, image_3.get());
    tex->setImage(4, image_4.get());

    //for (unsigned i = 0; i < _atlasImages.size(); ++i)
    //{
    //    osg::Image* image = _atlasImages[i].get();

    //    osg::ref_ptr<osg::Image> temp;

    //    // make sure the texture array is POT - required now for mipmapping to work
    //    if (s < 0)
    //    {
    //        s = osgEarth::nextPowerOf2(image->s());
    //        t = osgEarth::nextPowerOf2(image->t());
    //        tex->setTextureSize(s, t, _atlasImages.size());
    //    }

    //    if (image->s() != s || image->t() != t)
    //    {
    //        ImageUtils::resizeImage(image, s, t, temp);
    //    }
    //    else
    //    {
    //        temp = image;
    //    }

    //    tex->setImage(i, temp.get());
    //}

    //OE_INFO << LC << "Created atlas with " << _atlasImages.size() << " unique images" << std::endl;

    tex->setFilter(tex->MIN_FILTER, tex->NEAREST_MIPMAP_LINEAR);
    tex->setFilter(tex->MAG_FILTER, tex->LINEAR);
    tex->setWrap(tex->WRAP_S, tex->CLAMP_TO_BORDER);
    tex->setWrap(tex->WRAP_T, tex->CLAMP_TO_EDGE);
    tex->setUnRefImageDataAfterApply(Registry::instance()->unRefImageDataAfterApply().get());
    tex->setMaxAnisotropy(4.0);

    // Let the GPU do it since we only download this at startup
    tex->setUseHardwareMipMapGeneration(true);

    return tex;
}


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
        //VirtualProgram* vp = VirtualProgram::getOrCreate(stateset);
        //vp->setName(typeid(*this).name());

        // set light
        stateset->setDefine(OE_LIGHTING_DEFINE, "1", osg::StateAttribute::ON);
        stateset->setDefine("OE_NUM_LIGHTS", "1");
        stateset->setDefine("OE_USE_PBR", "1");
        stateset->setDefine("cascade", "1");

       



        MaterialURI matUri;
        PbrMaterial mat;
        std::string prefix = "oe_pbr.";
        stateset->getOrCreateUniform(prefix + "baseColorFactor", osg::Uniform::FLOAT_VEC4)->set(mat.baseColorFactor);
        stateset->getOrCreateUniform(prefix + "emissiveFactor", osg::Uniform::FLOAT_VEC3)->set(mat.baseColorFactor);
        stateset->getOrCreateUniform(prefix + "metallicFactor", osg::Uniform::FLOAT)->set(mat.metallicFactor);
        stateset->getOrCreateUniform(prefix + "roughnessFactor", osg::Uniform::FLOAT)->set(mat.roughnessFactor);
        stateset->getOrCreateUniform(prefix + "alphaMask", osg::Uniform::FLOAT)->set(mat.alphaMask);
        stateset->getOrCreateUniform(prefix + "alphaMaskCutoff", osg::Uniform::FLOAT)->set(mat.alphaMaskCutoff);
        stateset->getOrCreateUniform(prefix + "aoStrength", osg::Uniform::FLOAT)->set(mat.aoStrength);

        
        stateset->setDefine("OE_ENABLE_NORMAL_MAP", "0", osg::StateAttribute::ON);
        stateset->setDefine("OE_ENABLE_MR_MAP", "1", osg::StateAttribute::ON);
        stateset->setDefine("OE_ENABLE_AO_MAP", "2", osg::StateAttribute::ON);
        stateset->setDefine("OE_ENABLE_EMISSIVE_MAP", "3", osg::StateAttribute::ON);
        stateset->setDefine("OE_ENABLE_BASECOLOR_MAP", "4", osg::StateAttribute::ON);

        //stateset->setTextureAttributeAndModes(1, new osg::Texture2D(osgDB::readImageFile("D:/GitProject/FEngine/Assets/PbrBox/BoomBox_normal.png")), osg::StateAttribute::ON);
        //stateset->getOrCreateUniform("oe_normal_map", osg::Uniform::SAMPLER_2D)->set(1);
        //stateset->setTextureAttribute(1, new osg::TexEnv(osg::TexEnv::ADD));
        //stateset->setTextureAttributeAndModes(2, new osg::Texture2D(osgDB::readImageFile("D:/GitProject/FEngine/Assets/PbrBox/BoomBox_roughnessMetallic.png")), osg::StateAttribute::ON);
        //stateset->getOrCreateUniform("oe_mr_map", osg::Uniform::SAMPLER_2D)->set(2);
        //stateset->setTextureAttribute(2, new osg::TexEnv(osg::TexEnv::DECAL));
        //stateset->setTextureAttributeAndModes(3, new osg::Texture2D(osgDB::readImageFile("D:/GitProject/FEngine/Assets/PbrBox/BoomBox_occlusion.png")), osg::StateAttribute::ON);
        //stateset->getOrCreateUniform("oe_ao_map", osg::Uniform::SAMPLER_2D)->set(3);
        //stateset->setTextureAttribute(3, new osg::TexEnv(osg::TexEnv::ADD));
        //stateset->setTextureAttributeAndModes(4, new osg::Texture2D(osgDB::readImageFile("D:/GitProject/FEngine/Assets/PbrBox/BoomBox_emissive.png")), osg::StateAttribute::ON);
        //stateset->getOrCreateUniform("oe_emissive_map", osg::Uniform::SAMPLER_2D)->set(4);
        //stateset->setTextureAttribute(4, new osg::TexEnv(osg::TexEnv::REPLACE));


        osg::Texture* tex = createTextureAtlas();
        stateset->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);
        stateset->getOrCreateUniform("pbrMaps", osg::Uniform::SAMPLER_2D_ARRAY)->set(0);
       
       /* _atlas = tex;*/

        //取消或者置换这个stage

        // all the map share the same uv
       /* enableTextureUnit(stateset, 0, URI(matUri.baseColorMap), OE_ENABLE_BASECOLOR_MAP, oe_basecolor_map );
        enableTextureUnit(stateset, 1, URI(matUri.normalMap), OE_ENABLE_NORMAL_MAP, oe_normal_map);
        enableTextureUnit(stateset, 2, URI(matUri.metalRoughnessMap), OE_ENABLE_MR_MAP, oe_basecolor_map);
        enableTextureUnit(stateset, 3, URI(matUri.emissiveMap), OE_ENABLE_EMISSIVE_MAP, oe_emissive_map);
        enableTextureUnit(stateset, 4, URI(matUri.occulusionMap), OE_ENABLE_AO_MAP, oe_ao_map);*/


        BasicPbrShaders& shaders = PbrShadersFactory::get(false);
        VirtualProgram* pbrVP = VirtualProgram::getOrCreate(stateset);
        //auto functions = "..//OE_PBR/Shader//BRDF.glsl";
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

      
        std::cout << osgEarth::getAbsolutePath(shaderPath) << std::endl;
        //shaders.load(pbrVP, "material.glsl", dbo.get());
        shaders.load(pbrVP, "pbr.glsl", dbo.get());
        std::string materialInput =
            " struct pbr_Material                                                 \n"
            " {                                                                   \n"
            "     vec4  baseColorFactor;                                          \n"
            "     vec3  emissiveFactor;                                           \n"
            "     float metallicFactor;                                           \n"
            "     float roughnessFactor;                                          \n"
            "     float alphaMask;                                                \n"
            "     float alphaMaskCutoff;                                          \n"
            "     float aoStrength;                                               \n"
            " }oe_pbr;                                                            \n"
            " out pbr_Material oe_pbr;                                            \n"
            " void pbr_material_input(inout vec4 color)                           \n"
            " {                                                                   \n"
            "     oe_pbr.baseColorFactor = vec4(1.0, 1.0, 1.0, 1.0);              \n"
            "     oe_pbr.emissiveFactor = vec3(0.0, 0.0, 1.0);                    \n"
            "     oe_pbr.metallicFactor = 0.0;                                    \n"
            "     oe_pbr.roughnessFactor = 0.5;                                   \n"
            "     oe_pbr.alphaMask = 0.1;                                         \n"
            "     oe_pbr.alphaMaskCutoff = 0.1;                                   \n"
            "     oe_pbr.aoStrength = 0.1;                                        \n"
            " }                                                                   \n";
        URIContext context(dbo.get());
        std::string externalSource = URI("C:\\Users\\10677\\source\\repos\\OE_PBR\\OE_PBR\\Shader\\material.glsl", context).getString(dbo);
        //std::string materialInput = osgDB::readFile<std::string>(, dbo.get());
       /* pbrVP->setFunction(
            "pbr_material_input", externalSource, VirtualProgram::LOCATION_FRAGMENT_COLORING, 0.5f
        );*/
       
        
       


    }
}


void osgEarth::Util::PbrLightEffect::enableTextureUnit(osg::StateSet* stateset, 
    unsigned int texUnit, const osgEarth::URI& uri,const std::string& define, const std::string& texName)
{
    //osgEarth::URI imageURI(matUri.baseColorMap);
    //env.referrer);
    osg::ref_ptr<osg::Texture2D> tex;
    bool cachedTex = false;
    TextureCache* texCache = _texCache;
    // get
    if (texCache)
    {
        osgEarth::Threading::ScopedLock lock(*_texCache);

        auto texItr = texCache->find(uri.full());
        if (texItr != texCache->end())
        {
            tex = texItr->second;
            cachedTex = true;
        }
    }

    if (!tex.valid())
    {
      /*  tex = makeTextureFromModel(texture);*/
    }
    // put
    if (tex.valid())
    {
        if (texCache && !cachedTex)
        {
            ScopedMutexLock lock(*texCache);
            auto insResult = texCache->insert(TextureCache::value_type(uri.full(), tex));
            if (insResult.second)
            {
                // Some other loader thread beat us in the cache
                tex = insResult.first->second;
            }
        }
        stateset->setTextureAttributeAndModes(texUnit, tex.get(), osg::StateAttribute::ON);
        stateset->getOrCreateUniform(texName, osg::Uniform::SAMPLER_2D)->set(texUnit);
        stateset->setDefine(define);

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
