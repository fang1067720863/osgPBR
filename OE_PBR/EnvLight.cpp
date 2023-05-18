#include "EnvLight.h"
#include <osgDB/ReadFile>
#include<osg/ShapeDrawable>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/Options>
#include <osgEarth/FileUtils>

#include<osg/MatrixTransform>
#include"SnapImage.h"
#include"ddsNew.h"


   

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
    std::string path = "C:\\Users\\10677\\source\\repos\\OE_PBR\\OE_PBR\\Asset\\IBL";
    osg::ref_ptr<osgDB::Options> readOption = new osgDB::Options ("IBL");
    readOption->setDatabasePath(path);

    auto prefilterMapPath = "Cerberus_NSpecularHDR.dds";

    auto images = readCubeImages(prefilterMapPath, readOption.get());
    prefilterMap = new osg::TextureCubeMap();
    for (uint32_t i = 0; i<6;i++)
    {
        prefilterMap->setImage(i, images[i]);
    }
    
    prefilterMap->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
    prefilterMap->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
    prefilterMap->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);

    prefilterMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    prefilterMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    prefilterMap->setNumMipmapLevels(images[0]->getNumMipmapLevels());

    std::cout << prefilterMap->getTextureHeight() << std::endl;
    std::cout << prefilterMap->getTextureWidth() << std::endl;
    std::cout << prefilterMap->getNumMipmapLevels() << std::endl;
    std::cout << prefilterMap->getInternalFormatType() << std::endl;
    std::cout << prefilterMap->getSourceType() << std::endl;


    auto irridianceMapPath = "Cerberus_NDiffuseHDR.dds";
    auto irridianceImages = readCubeImages(irridianceMapPath, readOption.get());
    irridianceMap = new osg::TextureCubeMap();
    for (uint32_t i = 0; i < 6; i++)
    {
        irridianceMap->setImage(i, irridianceImages[i]);
    }
    irridianceMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    irridianceMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    irridianceMap->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

    irridianceMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    irridianceMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    irridianceMap->setNumMipmapLevels(1);


    auto brdfLUTPath = "Cerberus_NBrdf.dds";
    auto brdfImage = osgDB::readRefImageFile(brdfLUTPath, readOption.get());

    brdfLUTMap = new osg::Texture2D();
    brdfLUTMap->setImage(brdfImage.get());
    brdfLUTMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    brdfLUTMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

    brdfLUTMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    brdfLUTMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    brdfLUTMap->setNumMipmapLevels(1);

    
}
