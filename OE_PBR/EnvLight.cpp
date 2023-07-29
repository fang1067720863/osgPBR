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
//#include"SnapImage.h"




   

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

void EnvLightEffect::InitEnvMapAtlas()
{
    std::string path = "C:\\Users\\10677\\source\\repos\\OE_PBR\\OE_PBR\\Asset\\IBL\\pisaHDR";
    osg::ref_ptr<osgDB::Options> readOption = new osgDB::Options ("IBL");
    readOption->setDatabasePath(path);

    //auto prefilterMapPath = "Cerberus_NSpecularHDR.dds";

    //auto images = readCubeImages(prefilterMapPath, readOption.get());

    if (_useCubeUV)
    {
       // prefilterMap = new osg::TextureCubeMap();
       // for (uint32_t i = 0; i < 6; i++)
       // {
       //     prefilterMap->setImage(i, images[i]);
       // }

       // prefilterMap->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
       // prefilterMap->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
       // prefilterMap->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);

       // prefilterMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
       // prefilterMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
       //// prefilterMap->setNumMipmapLevels(images[0]->getNumMipmapLevels());

       // std::cout << prefilterMap->getTextureHeight() << std::endl;
       // std::cout << prefilterMap->getTextureWidth() << std::endl;
       // //std::cout << prefilterMap->getNumMipmapLevels() << std::endl;
       // std::cout << prefilterMap->getInternalFormatType() << std::endl;
       // std::cout << prefilterMap->getSourceType() << std::endl;


       // auto irridianceMapPath = "Cerberus_NDiffuseHDR.dds";
       // auto irridianceImages = readCubeImages(irridianceMapPath, readOption.get());
       // irridianceMap = new osg::TextureCubeMap();
       // for (uint32_t i = 0; i < 6; i++)
       // {
       //     irridianceMap->setImage(i, irridianceImages[i]);
       // }
       // irridianceMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
       // irridianceMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
       // irridianceMap->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

       // irridianceMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
       // irridianceMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
        //irridianceMap->setNumMipmapLevels(1);
    }
    else {
        auto prefilterMapPath = "specular.dds";
        prefilterMap = new osg::Texture2D();
        auto prefilterMapImage = osgDB::readRefImageFile(prefilterMapPath, readOption.get());
        osg::Texture2D* prefilterMap2D = dynamic_cast<osg::Texture2D*>(prefilterMap.get());
        prefilterMap2D->setImage(0,prefilterMapImage.get());
        prefilterMap2D->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        prefilterMap2D->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        prefilterMap2D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST_MIPMAP_LINEAR);
        prefilterMap2D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST_MIPMAP_LINEAR);
        prefilterMap2D->setMaxAnisotropy(4.0);
        osg::ref_ptr<osg::State> state = new osg::State;
        state->initializeExtensionProcs();
        prefilterMap2D->allocateMipmapLevels();
        prefilterMap2D->apply(*state);
        prefilterMap2D->setUseHardwareMipMapGeneration(false);
        prefilterMap = prefilterMap2D;

        auto envCubeMapPath = "env.dds";
        envCubeMap = new osg::Texture2D();
        auto envCubeMapImage = osgDB::readRefImageFile(envCubeMapPath, readOption.get());
        osg::Texture2D* envCubeMap2D = dynamic_cast<osg::Texture2D*>(envCubeMap.get());
        envCubeMap2D->setImage(0, prefilterMapImage.get());
        envCubeMap2D->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        envCubeMap2D->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        envCubeMap2D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST_MIPMAP_LINEAR);
        envCubeMap2D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST_MIPMAP_LINEAR);
        envCubeMap2D->setMaxAnisotropy(4.0);
        //osg::ref_ptr<osg::State> state = new osg::State;
        //state->initializeExtensionProcs();
        envCubeMap2D->allocateMipmapLevels();
        /*envCubeMap2D->apply(*state);*/
        envCubeMap2D->setUseHardwareMipMapGeneration(true);
        envCubeMap = envCubeMap2D;

        auto irridianceMapPath = "diffuse.png";
        irridianceMap = new osg::Texture2D();
        auto irridianceMapImage = osgDB::readRefImageFile(irridianceMapPath, readOption.get());
        osg::Texture2D* irridianceMap2D = dynamic_cast<osg::Texture2D*>(irridianceMap.get());
        irridianceMap2D->setImage(irridianceMapImage.get());
        irridianceMap2D->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        irridianceMap2D->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        irridianceMap2D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
        irridianceMap2D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
        irridianceMap = irridianceMap2D;
    }
   
   

    auto brdfLUTPath = "brdfLUT.png";
    auto brdfImage = osgDB::readRefImageFile(brdfLUTPath, readOption.get());

    brdfLUTMap = new osg::Texture2D();
    brdfLUTMap->setImage(brdfImage.get());
    brdfLUTMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    brdfLUTMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

    brdfLUTMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    brdfLUTMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    brdfLUTMap->setNumMipmapLevels(1);

    
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
        ss->getOrCreateUniform("envLightIntensity", osg::Uniform::FLOAT)->set(envLight->lightIntensity());
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
