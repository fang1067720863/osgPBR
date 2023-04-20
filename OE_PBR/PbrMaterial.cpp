


//author:1067720863@qq.com
//create date:2023/04
//decription: PBR Material
#include<Windows.h>
#include "PbrMaterial.h"

#include<osg/Texture2DArray>
#include <osgEarth/Common>
#include<osgEarth/Registry>
#include<osgEarth/Math>
#include<osgDB/ReadFile>


void osgEarth::StandardPBRMaterial::setTextureAttribute(TextureEnum mapEnum, const std::string& fileName, const std::string& defineName, StateAttribute::OverrideValue value)
{
	 
    if (_maps.find(mapEnum) == _maps.end())
    {
        _maps[mapEnum] = TextureInfo();
               
    }
    auto& info = _maps[mapEnum];
    info._path = fileName;
    info._defineKey = defineName.empty() ? getDefaultDefineName(mapEnum) : defineName;
    info._defineVal = std::to_string(1.0f*(int)mapEnum);
        
}

std::string osgEarth::StandardPBRMaterial::getDefaultDefineName(TextureEnum mapEnum)
{
    std::string name;
    switch (mapEnum)
    {
    case osgEarth::StandardPBRMaterial::BaseColorMap:
        name = "OE_ENABLE_BASECOLOR_MAP";
        break;
    case osgEarth::StandardPBRMaterial::EmissiveMap:
        name = "OE_ENABLE_EMISSIVE_MAP";
        break;
    case osgEarth::StandardPBRMaterial::OcclusionMap:
        name = "OE_ENABLE_AO_MAP";
        break;
    case osgEarth::StandardPBRMaterial::NormalMap:
        name = "OE_ENABLE_NORMAL_MAP";
        break;
    case osgEarth::StandardPBRMaterial::MetalRoughenssMap:
        name = "OE_ENABLE_MR_MAP";
        break;
    case osgEarth::StandardPBRMaterial::EnvMap:
        name = "OE_ENABLE_MR_MAP";
        break;
    case osgEarth::StandardPBRMaterial::CustomMap:
        name = "OE_ENABLE_MR_MAP";
        break;
    case osgEarth::StandardPBRMaterial::Undefined:
        name = "OE_ENABLE_MR_MAP";
        break;
    default:
        break;
    }
    return name;
}

bool osgEarth::StandardPBRMaterial::setTextures(const TextureMaps& maps)
{
    this->_maps = maps;
    return true;
}

void osgEarth::StandardPBRMaterial::apply(State& state) const
{
    OSG_NOTICE << "Warning: Material::apply(State&) - not supported." << std::endl;

    state.Color(mBaseColorFactor.r(), mBaseColorFactor.g(), mBaseColorFactor.b(), mBaseColorFactor.a());
}


void osgEarth::StandardPBRMaterial::setTextureEnable(TextureEnum mapEnum, StateAttribute::OverrideValue enable)
{
    for (unsigned int i = 0; i < this->getNumParents(); i++)
    {
        osg::StateSet* stateSet = getParent(i);
        const auto& info = _maps[mapEnum];
        stateSet->setDefine(info._defineKey, info._defineVal, enable);
    }

}

osg::Texture* osgEarth::StandardPBRMaterial::createTextureAtlas()
{
    // Creates a texture array containing all the billboard images.
    // Each image is included only once.
    osg::Texture2DArray* tex = new osg::Texture2DArray();

    float s = -1.0f, t = -1.0f;
    size_t imageSize = _maps.size();
    //std::cout << "image" << imageSize << std::endl;
    tex->setTextureDepth(imageSize);
    unsigned int layer = 0;

    for (auto iter = _maps.begin(); iter != _maps.end(); iter++)
    {
        auto& info = iter->second;

        osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(info._path);
        info._imageValid = image.valid();
        osg::ref_ptr<osg::Image> temp;

        if (image.valid() == false)
        {
            continue;
        }


        if (s < 0)
        {
            s = osgEarth::nextPowerOf2(image->s());
            t = osgEarth::nextPowerOf2(image->t());
            tex->setTextureSize(s, t, imageSize);
        }

        if (image->s() != s || image->t() != t)
        {
            ImageUtils::resizeImage(image, s, t, temp);
            //std::cout << "image" << image->getFileName() << std::endl;
        }
        else
        {
            temp = image;
            //std::cout << "temp" << temp->getFileName()<<std::endl;
        }
        info._defineVal = std::to_string(layer * 1.0f);
        //std::cout << "info._defineVal layer" << layer << std::endl;
        tex->setImage(layer, temp.get());
        layer++;
    }
    //std::cout << "layer count" << layer << std::endl;



   // OE_INFO << LC << "Created atlas with " << _atlasImages.size() << " unique images" << std::endl;

    tex->setFilter(tex->MIN_FILTER, tex->NEAREST_MIPMAP_LINEAR);
    tex->setFilter(tex->MAG_FILTER, tex->NEAREST_MIPMAP_LINEAR);
    tex->setWrap(tex->WRAP_S, tex->REPEAT);
    tex->setWrap(tex->WRAP_T, tex->REPEAT);
    tex->setUnRefImageDataAfterApply(osgEarth::Registry::instance()->unRefImageDataAfterApply().get());
    tex->setMaxAnisotropy(4.0);

    // Let the GPU do it since we only download this at startup
    tex->setUseHardwareMipMapGeneration(true);

    return tex;
}

void osgEarth::PBRMaterialCallback::operator()(osg::StateAttribute* attr, osg::NodeVisitor* nv)
{

    static const std::string BASECOLOR = UPREFIX "baseColorFactor";
    static const std::string EMISSIVE = UPREFIX "emissiveFactor";
    static const std::string METALLIC = UPREFIX "metallicFactor";
    static const std::string ROUGHNESS = UPREFIX "roughnessFactor";
    static const std::string ALPHAMASK = UPREFIX "alphaMask";
    static const std::string ALPHAMASKCUTOFF = UPREFIX "alphaMaskCutoff";
    static const std::string AMBIENTOCCLUSION = UPREFIX "aoStrength";


    osgEarth::StandardPBRMaterial* material = static_cast<osgEarth::StandardPBRMaterial*>(attr);
    for (unsigned int i = 0; i < attr->getNumParents(); i++)
    {
        osg::StateSet* stateSet = attr->getParent(i);

        stateSet->getOrCreateUniform(BASECOLOR, osg::Uniform::FLOAT_VEC4)->set(material->getBaseColorFactor());
        stateSet->getOrCreateUniform(EMISSIVE, osg::Uniform::FLOAT_VEC3)->set(material->getEmissiveFactor());
        stateSet->getOrCreateUniform(METALLIC, osg::Uniform::FLOAT)->set(material->getMetallicFactor());
        stateSet->getOrCreateUniform(ROUGHNESS, osg::Uniform::FLOAT)->set(material->getRoughnessFactor());
        stateSet->getOrCreateUniform(ALPHAMASK, osg::Uniform::FLOAT)->set(material->getAlphaMask());
        stateSet->getOrCreateUniform(ALPHAMASKCUTOFF, osg::Uniform::FLOAT)->set(material->getAlphaMaskCutoff());
        stateSet->getOrCreateUniform(AMBIENTOCCLUSION, osg::Uniform::FLOAT)->set(material->getAoStrength());

        osg::Texture* tex = material->createTextureAtlas();
        unsigned int unit = 0;
        stateSet->setTextureAttributeAndModes(unit, tex, osg::StateAttribute::ON);
        stateSet->getOrCreateUniform("pbrMaps", osg::Uniform::SAMPLER_2D_ARRAY)->set(unit);

        for (auto iter = material->_maps.begin(); iter != material->_maps.end(); iter++)
        {
            auto textureInfo = iter->second;
            auto enable = textureInfo._imageValid ? osg::StateAttribute::ON : osg::StateAttribute::OFF;
            stateSet->setDefine(textureInfo._defineKey, textureInfo._defineVal, enable);
        }

    }
}
