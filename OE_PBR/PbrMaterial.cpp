


//author:1067720863@qq.com
//create date:2023/04
//decription: PBR Material
#include<Windows.h>
#include "PbrMaterial.h"


#include <osgEarth/Common>
#include<osgEarth/Registry>
#include<osgEarth/Math>
#include<osgDB/ReadFile>
#include"EnvLight.h"

osgEarth::StandardPBRMaterial::StandardPBRMaterial()
{
    mTextureAtlas = new osg::Texture2DArray();
    mTextureAtlas->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    mTextureAtlas->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
    mTextureAtlas->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
    mTextureAtlas->setWrap(osg::Texture2D:: WRAP_T, osg::Texture2D::REPEAT);
    mTextureAtlas->setUnRefImageDataAfterApply(true);
    mTextureAtlas->setResizeNonPowerOfTwoHint(false);
}

//void osgEarth::StandardPBRMaterial::setMaterialImage(osg::Image* image, TextureEnum mapEnum)
//{
//   
//}

//void osgEarth::StandardPBRMaterial::setTextureAttribute(TextureEnum mapEnum, const std::string& fileName, const std::string& defineName, StateAttribute::OverrideValue value)
//{
//	 
//    if (_maps.find(mapEnum) == _maps.end())
//    {
//        _maps[mapEnum] = TextureInfo();
//               
//    }
//    auto& info = _maps[mapEnum];
//    info._path = fileName;
//    info._defineKey = defineName.empty() ? getDefaultDefineName(mapEnum) : defineName;
//    info._defineVal = std::to_string(1.0f*(int)mapEnum);
//        
//}

std::string osgEarth::StandardPBRMaterial::getDefaultDefineName(TextureEnum mapEnum)
{
    if (textureDefines.find(mapEnum) == textureDefines.end())
    {
        return "";
    }
    return textureDefines.at(mapEnum);
}

//bool osgEarth::StandardPBRMaterial::setTextures(const TextureMaps& maps)
//{
//    this->_maps = maps;
//    return true;
//}


void osgEarth::StandardPBRMaterial::apply(State& state) const
{
    OSG_NOTICE << "Warning: Material::apply(State&) - not supported." << std::endl;

    state.Color(mBaseColorFactor.r(), mBaseColorFactor.g(), mBaseColorFactor.b(), mBaseColorFactor.a());
}

void osgEarth::StandardPBRMaterial::setMaterialImage(TextureEnum mapEnum, osg::Image* image)
{
    int layer = _maps.size();
    if (_maps.find(mapEnum) != _maps.end())
    {
        layer = _maps[mapEnum].layer;
    }
    osg::ref_ptr<osg::Image> temp;
    if (texWidth < 0 && texHeight < 0)
    {
        texWidth = image->s();
        texHeight = image->t();
        temp = image;
    }
    else {
        if (texWidth != image->s() || texHeight != image->t())
        {
            ImageUtils::resizeImage(image, texWidth, texHeight, temp);
        }
        else {
            temp = image;
        }
    }
   
    std::cout << "dff" << temp->s() << " " << temp->t() << " " << temp->r()<<" " << layer << std::endl;
    mTextureAtlas->setImage(layer, temp.get());
    _maps[mapEnum] = TextureInfo{ layer };
}

void osgEarth::StandardPBRMaterial::setMaterialImage(TextureEnum mapEnum, const std::string& imageUrl)
{
    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(imageUrl, _options.get());

    if (image.valid() == false)
    {
        return;
    }
    setMaterialImage(mapEnum, image.get());
}


//void osgEarth::StandardPBRMaterial::setTextureEnable(TextureEnum mapEnum, StateAttribute::OverrideValue enable)
//{
//    for (unsigned int i = 0; i < this->getNumParents(); i++)
//    {
//        osg::StateSet* stateSet = getParent(i);
//        const auto& info = _maps[mapEnum];
//        stateSet->setDefine(info._defineKey, info._defineVal, enable);
//    }
//
//}

osg::Texture* osgEarth::StandardPBRMaterial::createTexture(const std::string& imagePath)
{
    float s = -1.0f, t = -1.0f;

    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(imagePath, _options.get());

    if (image.valid() == false)
    {
        return nullptr;
    }
    osg::Texture2D* tex = new osg::Texture2D();

    tex->setFilter(tex->MIN_FILTER, tex->NEAREST_MIPMAP_LINEAR);
    tex->setFilter(tex->MAG_FILTER, tex->NEAREST_MIPMAP_LINEAR);
    tex->setWrap(tex->WRAP_S, tex->REPEAT);
    tex->setWrap(tex->WRAP_T, tex->REPEAT);
    tex->setUnRefImageDataAfterApply(osgEarth::Registry::instance()->unRefImageDataAfterApply().get());
    tex->setMaxAnisotropy(4.0);
    tex->setImage(image.get());
    // Let the GPU do it since we only download this at startup
    tex->setUseHardwareMipMapGeneration(true);

    return tex;
}

//osg::Texture* osgEarth::StandardPBRMaterial::createTextureAtlas()
//{
//    // Creates a texture array containing all the billboard images.
//    // Each image is included only once.
//    osg::Texture2DArray* tex = new osg::Texture2DArray();
//
//    float s = -1.0f, t = -1.0f;
//    size_t imageSize = _maps.size();
//    //std::cout << "image" << imageSize << std::endl;
//    tex->setTextureDepth(imageSize);
//    unsigned int layer = 0;
//
//    for (auto iter = _maps.begin(); iter != _maps.end(); iter++)
//    {
//        auto& info = iter->second;
//
//        osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(info._path, _options.get());
//        info._imageValid = image.valid();
//        //osg::ref_ptr<osg::Image> temp;
//
//        if (image.valid() == false)
//        {
//            std::cout << "info._path" << info._path << "is not valid!" << std::endl;
//            continue;
//        }
//
//        std::cout << info._path << " valid" << std::endl;
//        info._defineVal = std::to_string(layer * 1.0f);
//        //std::cout << "info._defineVal layer" << layer << std::endl;
//        tex->setImage(layer, image.get());
//        layer++;
//    }
//    //std::cout << "layer count" << layer << std::endl;
//
//
//
//   // OE_INFO << LC << "Created atlas with " << _atlasImages.size() << " unique images" << std::endl;
//
//    tex->setFilter(tex->MIN_FILTER, tex->LINEAR);
//    tex->setFilter(tex->MAG_FILTER, tex->LINEAR);
//    tex->setWrap(tex->WRAP_S, tex->REPEAT);
//    tex->setWrap(tex->WRAP_T, tex->REPEAT);
//    tex->setUnRefImageDataAfterApply(true);
//    tex->setResizeNonPowerOfTwoHint(false);
//
//    return tex;
//}

void osgEarth::PBRMaterialCallback::operator()(osg::StateAttribute* attr, osg::NodeVisitor* nv)
{

    static const std::string BASECOLOR = UPREFIX "baseColorFactor";
    static const std::string EMISSIVE = UPREFIX "emissiveFactor";
    static const std::string METALLIC = UPREFIX "metallicFactor";
    static const std::string ROUGHNESS = UPREFIX "roughnessFactor";
    static const std::string ALPHAMASK = UPREFIX "alphaMask";
    static const std::string ALPHAMASKCUTOFF = UPREFIX "alphaMaskCutoff";
    static const std::string AMBIENTOCCLUSION = UPREFIX "aoStrength";
    static const std::string RECEIVEENVLIGHT = UPREFIX "aoStrength";

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
        
        osg::Texture* tex = material->getTextureAtlas();
        int unit = material->getOrCreateTexUnit("pbrMaps");
        stateSet->setTextureAttributeAndModes(unit, tex, osg::StateAttribute::ON);
        stateSet->getOrCreateUniform("pbrMaps", osg::Uniform::SAMPLER_2D_ARRAY)->set(unit);


        for (const auto iter : material->_maps)
        {
            auto define = material->getDefaultDefineName(iter.first);
            float layer = iter.second.layer * 1.0f;
            stateSet->setDefine(define, std::to_string(layer), osg::StateAttribute::ON);
            
        }
     

    }
}

void osgEarth::ExtensionedMaterialCallback::operator()(osg::StateAttribute* attr, osg::NodeVisitor* nv)
{
    PBRMaterialCallback::operator()(attr, nv); /*
    osgEarth::ExtensionedMaterial* material = static_cast<osgEarth::ExtensionedMaterial*>(attr);
    for (unsigned int i = 0; i < attr->getNumParents(); i++)
    {
        const auto& maps = material->customMaps();
        auto iter = maps.begin();
        material->incementTexUnit();
        for (; iter != maps.end(); iter++)
        {
            osg::StateSet* stateSet = attr->getParent(i);
            auto uniformKey = iter->first;

            osg::Texture* tex = material->createTexture(iter->second._path);
            stateSet->setTextureAttributeAndModes(material->texUnitCnt(), tex, osg::StateAttribute::ON);
            stateSet->getOrCreateUniform(uniformKey, osg::Uniform::SAMPLER_2D)->set(material->texUnitCnt());
            material->incementTexUnit();
            
            auto textureInfo = iter->second;
            stateSet->setDefine(textureInfo._defineKey, textureInfo._defineVal, osg::StateAttribute::ON);
        }
    }*/
}
