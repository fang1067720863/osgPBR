#pragma once

//author:1067720863@qq.com
//create date:2023/04
//decription: PBR Material

#include <set>
#include <osg/Node>
#include <osg/NodeVisitor>
#include <osg/LightSource>
#include <osg/Light>
#include<osg/Texture>
#include<osg/Vec4>
#include<osg/State>
#include<osg/StateAttribute>

#include <osgEarth/Common>
#include <osgEarth/Threading>
#include"Export.h"



#define PROPERTY_DEFAULT(T,var, default) public: \
    inline void set##var(const T& _var){m##var=_var;}\
    T get##var() const{return m##var;}\
    protected: T m##var{default};
#define UPREFIX "oe_pbr."



namespace osgEarth {
    using namespace osg;
    #define OE_MATERIAL (osg::StateAttribute::Type)(osg::StateAttribute::MATERIAL)
    class OE_MATERIAL_PULGIN StandardPBRMaterial : public osg::StateAttribute
    {
        friend class PBRMaterialCallback;
    public:
        enum TextureEnum
        {
            BaseColorMap = 0,  
            EmissiveMap = 1,  
            OcclusionMap = 2,  
            NormalMap = 3,
            MetalRoughenssMap = 4,   
            EnvMap = 5,
            CustomMap = 6,
            Undefined = 7
        };
        enum class AlphaMode {
            Opaque = 0,
            Blend =1,
            Mask =2 
        };
        struct TextureInfo
        {
            std::string _defineKey;
            std::string _defineVal;
            std::string _path;
            bool _imageValid{ false };
        };
        using TextureMaps = std::map<TextureEnum, TextureInfo>;
    
        StandardPBRMaterial(){}

        StandardPBRMaterial(const StandardPBRMaterial& mat, const CopyOp& copyop = CopyOp::SHALLOW_COPY) :
            StateAttribute(mat, copyop) {}
#ifdef META_StateAttribute
        META_StateAttribute(osgEarth, StandardPBRMaterial, OE_MATERIAL)

#endif // META_StateAttribute

        virtual int compare(const StateAttribute& sa) const
        {
            return 0;  
        }

        StandardPBRMaterial& operator = (const StandardPBRMaterial& rhs) {
            if (&rhs == this) return *this; return *this;
        }

        virtual void apply(State& state) const;
       
        void setTextureAttribute(TextureEnum mapEnum, const std::string& fileName, const std::string& defineName = "", StateAttribute::OverrideValue value = StateAttribute::ON);
        void setTextureEnable(TextureEnum mapEnum, StateAttribute::OverrideValue enable);
        bool setTextures(const TextureMaps& maps);

        PROPERTY_DEFAULT(Vec4, BaseColorFactor, Vec4(1.0, 1.0, 1.0, 1.0))
        PROPERTY_DEFAULT(Vec3, EmissiveFactor, Vec3(0.1, 0.1, 0.1))
        PROPERTY_DEFAULT(float, RoughnessFactor, 0.1)
        PROPERTY_DEFAULT(float, MetallicFactor, 0.1)
        PROPERTY_DEFAULT(float, AlphaMask, 0.1)
        PROPERTY_DEFAULT(AlphaMode, AlphaMode, AlphaMode::Blend)
        PROPERTY_DEFAULT(float, AlphaMaskCutoff, 0.2)
        PROPERTY_DEFAULT(float, AoStrength, 0.1)

        PROPERTY_DEFAULT(bool,ReceiveEnvLight, true)

    
    protected:
        virtual ~StandardPBRMaterial() {};
        osg::Texture* createTextureAtlas();

    private:
        std::string getDefaultDefineName(TextureEnum mapEnum);

        TextureMaps _maps;
       
    };

    class OE_MATERIAL_PULGIN PBRMaterialCallback : public osg::StateAttributeCallback {
    public:
        virtual void operator()(osg::StateAttribute* attr, osg::NodeVisitor* nv);

    };
}



