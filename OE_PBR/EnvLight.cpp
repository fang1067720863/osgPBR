#include "EnvLight.h"
#include <osgDB/ReadFile>
#include<osg/ShapeDrawable>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/Options>
#include <osgEarth/FileUtils>
#include<osg/Texture2DArray>

#include<osg/MatrixTransform>
#include"PbrMaterial.h"




   

osg::ref_ptr<EnvLightEffect>& EnvLightEffect::instance()
{
    static osg::ref_ptr<EnvLightEffect> s_registry = new EnvLightEffect();
  /*  if (erase)
    {
        s_registry->destruct();
        s_registry = 0;
    }*/
    return s_registry; // will return NULL on erase
}

EnvLightEffect::EnvLightEffect()
{

}

EnvLightEffect::EnvLightEffect(const std::string& envCubeFile) : _enable(false), _texturesReady(false)
{
   
}
    
void EnvLightEffect::setEnable(bool enable)
{
    _enable = enable;
    if (enable && !_texturesReady) _ResetEnvMapAtlas();
    _lightIntensity = 0.5;
}

void EnvLightEffect::setEnvMapAtlas(const EnvMapConfig& config, osgDB::Options* opts)
{
    _config = config;
    if (opts != nullptr)
    {
        _options = opts;
    }
   
    _ResetEnvMapAtlas();
}

void EnvLightEffect::_ResetEnvMapAtlas()
{
    
    auto readOption = _options;

    auto prefilterMapPath = _config._specularMap;
    prefilterMap = new osg::Texture2D();
    auto prefilterMapImage = osgDB::readRefImageFile(prefilterMapPath, readOption);
    if (!prefilterMapImage.valid())
    {
        return;
    }
    osg::Texture2D* prefilterMap2D = dynamic_cast<osg::Texture2D*>(prefilterMap.get());

    std::cout << "miplevel" << prefilterMapImage->getNumMipmapLevels() << std::endl;
    _maxReflectionLOD = prefilterMapImage->getNumMipmapLevels();

    prefilterMap2D->setNumMipmapLevels(_maxReflectionLOD);
    prefilterMap2D->setImage(0,prefilterMapImage.get());
    prefilterMap2D->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    prefilterMap2D->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    prefilterMap2D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST_MIPMAP_LINEAR);
    prefilterMap2D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST_MIPMAP_LINEAR);
    prefilterMap2D->setMaxAnisotropy(4.0);
    prefilterMap2D->allocateMipmapLevels();
    prefilterMap2D->setUseHardwareMipMapGeneration(false);
    prefilterMap = prefilterMap2D;


    auto envCubeMapPath = _config._envMap;
    envCubeMap = new osg::Texture2D();
    auto envCubeMapImage = osgDB::readRefImageFile(envCubeMapPath, readOption);
    osg::Texture2D* envCubeMap2D = dynamic_cast<osg::Texture2D*>(envCubeMap.get());
    envCubeMap2D->setImage(0, prefilterMapImage.get());
    envCubeMap2D->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    envCubeMap2D->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    envCubeMap2D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    envCubeMap2D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST_MIPMAP_LINEAR);
    envCubeMap2D->setMaxAnisotropy(4.0);
    envCubeMap2D->allocateMipmapLevels();
    envCubeMap2D->setUseHardwareMipMapGeneration(true);
    envCubeMap = envCubeMap2D;

    auto irridianceMapPath = _config._diffuseMap;
    irridianceMap = new osg::Texture2D();
    auto irridianceMapImage = osgDB::readRefImageFile(irridianceMapPath, readOption);
    osg::Texture2D* irridianceMap2D = dynamic_cast<osg::Texture2D*>(irridianceMap.get());
    irridianceMap2D->setImage(irridianceMapImage.get());
    irridianceMap2D->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    irridianceMap2D->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    irridianceMap2D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    irridianceMap2D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    irridianceMap = irridianceMap2D;
   
    auto brdfLUTPath = "brdfLUT.png";
    auto brdfImage = osgDB::readRefImageFile(brdfLUTPath, readOption);

    brdfLUTMap = new osg::Texture2D();
    brdfLUTMap->setImage(brdfImage.get());
    brdfLUTMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    brdfLUTMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

    brdfLUTMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    brdfLUTMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    brdfLUTMap->setNumMipmapLevels(1);
    
    if (!replaceCallbacks.empty())
    {
        for (auto& cb : replaceCallbacks)
        {
            cb();
        }
    }
  
}

EnvLightGL3UniformGenerator::EnvLightGL3UniformGenerator():
_statesetsMutex("EnvLightGL3UniformGenerator(OE)")
{
}

bool EnvLightGL3UniformGenerator::run(osg::Object* obj, osg::Object* data)
{
    EnvLightSource* lightSource = dynamic_cast<EnvLightSource*>(obj);
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(data);

    if (cv && lightSource )
    {
        auto envLight = lightSource->getEnvLightEffect()->instance();

        osg::StateSet* ss = cv->getCurrentRenderStage()->getStateSet();
        if (ss == 0L)
        {
            cv->getCurrentRenderStage()->setStateSet(ss = new osg::StateSet());

            Threading::ScopedMutexLock lock(_statesetsMutex);
            _statesets.push_back(ss);
        }
        auto material = ss->getAttribute(osg::StateAttribute::MATERIAL);
        if (dynamic_cast<osgEarth::StandardPBRMaterial*>(material))
        {
            auto useCubeUV = envLight->useCubeUV() ? osg::StateAttribute::ON : osg::StateAttribute::OFF;
            ss->setDefine("USE_ENV_MAP");

            auto uniformType = envLight->useCubeUV() ? osg::Uniform::SAMPLER_CUBE : osg::Uniform::SAMPLER_2D;
            ss->setDefine("USE_ENV_CUBE_UV", useCubeUV);

            ss->getOrCreateUniform("envLightIntensity", osg::Uniform::FLOAT)->set(envLight->lightIntensity());
            ss->getOrCreateUniform("MAX_REFLECTION_LOD", osg::Uniform::FLOAT)->set(envLight->maxReflectionLOD());



            int unit = 5;
            osg::Texture* diffuseEnvMap = envLight->getIrridianceMap();
            ss->setTextureAttributeAndModes(unit, diffuseEnvMap, osg::StateAttribute::ON);
            ss->getOrCreateUniform("irradianceMap", uniformType)->set(unit++);

            osg::Texture* specularEnvMap = envLight->getPrefilterMap();
            ss->setTextureAttributeAndModes(unit, specularEnvMap, osg::StateAttribute::ON);
            ss->getOrCreateUniform("prefilterMap", osg::Uniform::SAMPLER_2D)->set(unit++);


            osg::Texture* brdfLUTMap = envLight->getBrdfLUTMap();
            ss->setTextureAttributeAndModes(unit, brdfLUTMap, osg::StateAttribute::ON);
            ss->getOrCreateUniform("brdfLUT", osg::Uniform::SAMPLER_2D)->set(unit++);
        }


        
    }
    return traverse(obj, data);

}

void EnvLightGL3UniformGenerator::resizeGLBufferObjects(unsigned maxSize)
{
    Threading::ScopedMutexLock lock(_statesetsMutex);
    for (unsigned i = 0; i < _statesets.size(); ++i)
        _statesets[i]->resizeGLObjectBuffers(maxSize);
}

void EnvLightGL3UniformGenerator::releaseGLObjects(osg::State* state) const
{
    Threading::ScopedMutexLock lock(_statesetsMutex);
    for (unsigned i = 0; i < _statesets.size(); ++i)
        _statesets[i]->releaseGLObjects(state);
    _statesets.clear();
}

osg::ref_ptr<EnvLightEffect>& EnvLightSource::getEnvLightEffect()
{
     
    static osg::ref_ptr<EnvLightEffect> s_registry = new EnvLightEffect();
    /*  if (erase)
        {
            s_registry->destruct();
            s_registry = 0;
        }*/
    return s_registry; // will return NULL on erase
    
}

EnvLightSource::~EnvLightSource()
{
}

GenerateEnvLightUniforms::GenerateEnvLightUniforms():osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
{
    setNodeMaskOverride(~0);
}

void GenerateEnvLightUniforms::apply(osg::Node& node)
{
    osg::StateSet* ss = node.getStateSet();
    if (ss)
    {
        if (_statesets.find(ss) == _statesets.end())
        {
            const osg::StateSet::RefAttributePair* rap = ss->getAttributePair(osg::StateAttribute::MATERIAL);
            if (rap)
            {
                osgEarth::StandardPBRMaterial* material = dynamic_cast<osgEarth::StandardPBRMaterial*>(rap->first.get());
                if (material)
                {
                    if (material->getUpdateCallback())
                    {
                        return;
                    }
                    material->setUpdateCallback(new EnvLightGL3UniformCallback());  
                }

                // mark this stateset as visited.
                _statesets.insert(ss);
            }
        }
    }
    traverse(node);
}

void EnvLightGL3UniformCallback::operator()(osg::StateAttribute* attr, osg::NodeVisitor* nv)
{
    for (unsigned int i = 0; i < attr->getNumParents(); i++)
    {
        osg::StateSet* ss = attr->getParent(i);

        auto envLight = EnvLightEffect::instance();


        auto useCubeUV = envLight->useCubeUV() ? osg::StateAttribute::ON : osg::StateAttribute::OFF;
        ss->setDefine("USE_ENV_MAP");

        auto uniformType = envLight->useCubeUV() ? osg::Uniform::SAMPLER_CUBE : osg::Uniform::SAMPLER_2D;
        ss->setDefine("USE_ENV_CUBE_UV", useCubeUV);

        ss->getOrCreateUniform("envLightIntensity", osg::Uniform::FLOAT)->set(envLight->lightIntensity());
        ss->getOrCreateUniform("MAX_REFLECTION_LOD", osg::Uniform::FLOAT)->set(envLight->maxReflectionLOD());



        int unit = 5;
        osg::Texture* diffuseEnvMap = envLight->getIrridianceMap();
        ss->setTextureAttributeAndModes(unit, diffuseEnvMap, osg::StateAttribute::ON);
        ss->getOrCreateUniform("irradianceMap", uniformType)->set(unit++);

        osg::Texture* specularEnvMap = envLight->getPrefilterMap();
        ss->setTextureAttributeAndModes(unit, specularEnvMap, osg::StateAttribute::ON);
        ss->getOrCreateUniform("prefilterMap", osg::Uniform::SAMPLER_2D)->set(unit++);


        osg::Texture* brdfLUTMap = envLight->getBrdfLUTMap();
        ss->setTextureAttributeAndModes(unit, brdfLUTMap, osg::StateAttribute::ON);
        ss->getOrCreateUniform("brdfLUT", osg::Uniform::SAMPLER_2D)->set(unit++);
    }
   
}
