#include "PbrLightEffect.h"

#include <osgEarth/PhongLightingEffect>
#include <osgEarth/Registry>
#include <osgEarth/Capabilities>
#include <osgEarth/Shaders>
#include <osgEarth/Lighting>
#include<osgEarth/PhongLightingEffect>
#include<osgDB/FileUtils>
#include <osgEarth/FileUtils>
#include <osgDB/FileNameUtils>

using namespace osgEarth;
using namespace osgEarth::Util;


ShadersGL3 osgEarth::Util::PbrShadersFactory::s_gl3;
ShadersGL4 osgEarth::Util::PbrShadersFactory::s_gl4;

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
        stateset->setDefine(OE_LIGHTING_DEFINE, osg::StateAttribute::ON);
        stateset->setDefine("OE_NUM_LIGHTS", "1");
        stateset->setDefine("OE_USE_PBR", "1");

        stateset->setDefine("OE_ENABLE_BASECOLOR_MAP", osg::StateAttribute::OFF);
        stateset->setDefine("OE_ENABLE_NORMAL_MAP", osg::StateAttribute::OFF);
        stateset->setDefine("OE_ENABLE_MR_MAP", osg::StateAttribute::OFF);
        stateset->setDefine("OE_ENABLE_AO_MAP", osg::StateAttribute::OFF);
        stateset->setDefine("OE_ENABLE_EMISSIVE_MAP", osg::StateAttribute::OFF);



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

       
        enableTextureUnit(stateset, 0, URI(matUri.baseColorMap), OE_ENABLE_BASECOLOR_MAP, oe_basecolor_map );
        enableTextureUnit(stateset, 1, URI(matUri.normalMap), OE_ENABLE_NORMAL_MAP, oe_normal_map);
        enableTextureUnit(stateset, 2, URI(matUri.metalRoughnessMap), OE_ENABLE_MR_MAP, oe_basecolor_map);
        enableTextureUnit(stateset, 3, URI(matUri.emissiveMap), OE_ENABLE_EMISSIVE_MAP, oe_emissive_map);
        enableTextureUnit(stateset, 4, URI(matUri.occulusionMap), OE_ENABLE_AO_MAP, oe_ao_map);


        BasicPbrShaders& shaders = PbrShadersFactory::get(false);
        VirtualProgram* pbrVP = VirtualProgram::getOrCreate(stateset);
        //auto functions = "..//OE_PBR/Shader//BRDF.glsl";
        auto shaderPath = "..//..//OE_PBR//Shader";
        auto shaderPath2 = "..//OE_PBR//Shader";
        //auto texPath1 = osgEarth::getAbsolutePath(functions);
       // std::cout << texPath1 << texPath2<< std::endl;
        

        //std::string vert = shaders.vert();
        //std::string frag = shaders.brdf()+ shaders.frag();

        //pbrVP->setName("PBR");
        //pbrVP->setFunction(
        //    "vertex_main_pbr",
        //    vert,
        //    VirtualProgram::LOCATION_VERTEX_VIEW,
        //    1.1f);

        //pbrVP->setFunction(
        //    "fragment_main_pbr",
        //    frag,
        //    VirtualProgram::LOCATION_FRAGMENT_LIGHTING,
        //    0.8f);
        osg::ref_ptr<osgDB::Options> dbo = new osgDB::Options();
        
        
        if (osgDB::fileExists(osgEarth::getAbsolutePath(shaderPath)))
        {
            dbo->setDatabasePath(osgEarth::getAbsolutePath(shaderPath));
        }
        else {
            dbo->setDatabasePath(osgEarth::getAbsolutePath(shaderPath2));
        }

      
        std::cout << osgEarth::getAbsolutePath(shaderPath) << std::endl;
        shaders.load(pbrVP, "material.glsl", dbo.get());
        shaders.load(pbrVP, "pbr.glsl", dbo.get());
        
       /* pbrVP->setFunction(
            "pbr_material_input", discardAlpha, VirtualProgram::LOCATION_FRAGMENT_LIGHTING, 0.0f
        );
       */
        
       


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
