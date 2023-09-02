


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

std::string osgEarth::StandardPBRMaterial::getDefaultDefineName(TextureEnum mapEnum)
{
    if (textureDefines.find(mapEnum) == textureDefines.end())
    {
        return "";
    }
    return textureDefines.at(mapEnum);
}


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
   PBRMaterialCallback::operator()(attr, nv); 
   osgEarth::ExtensionedMaterial* material = static_cast<osgEarth::ExtensionedMaterial*>(attr);
    for (unsigned int i = 0; i < attr->getNumParents(); i++)
    {
        const auto& maps = material->_customMaps;
        auto iter = maps.begin();
        osg::StateSet* stateSet = attr->getParent(i);
        for (; iter != maps.end(); iter++)
        {
            auto uniformKey = iter->first;
            //osg::Texture* tex = material->createTexture("grass/metallic.png");
            int unit = material->getOrCreateTexUnit(uniformKey);
            stateSet->setTextureAttributeAndModes(unit, iter->second, osg::StateAttribute::ON);
            stateSet->getOrCreateUniform(uniformKey, osg::Uniform::SAMPLER_2D)->set(unit);

        }
        for (const auto& define : material->customDefines())
        {
            stateSet->setDefine(define, "1", osg::StateAttribute::ON);
        }
    }
}

void osgEarth::ExtensionedMaterial::extTextureAttribute(const std::string name, const std::string& fileName, const std::string& defineName, unsigned int uvChannel, StateAttribute::OverrideValue value)
{
    //osg::ref_ptr<osg::Texture> tex= 
    _customMaps[name] = createTexture(fileName);
    _customDefines.push_back(defineName);

}
