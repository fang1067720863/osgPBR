#pragma once
//author:1067720863@qq.com
//create date:2023/04
//decription: Cook_BRDF Light
#ifndef OSGEARTH_PBR_LIGHTING_EFFECT_H
#define OSGEARTH_PBR_LIGHTING_EFFECT_H
#include<Windows.h>

#include <osg/StateSet>
#include<osg/Texture2D>
#include <osg/Uniform>
#include <osg/observer_ptr>


#include <osgEarth/Common>
#include<osgEarth/Threading>
#include<osgEarth/URI>
#include <osgEarth/ShaderLoader>
#include"Export.h"




#define OE_ENABLE_BASECOLOR_MAP "OE_ENABLE_BASECOLOR_MAP"
#define OE_ENABLE_NORMAL_MAP "OE_ENABLE_NORMAL_MAP"
#define OE_ENABLE_MR_MAP "OE_ENABLE_MR_MAP"
#define OE_ENABLE_AO_MAP "OE_ENABLE_AO_MAP"
#define OE_ENABLE_EMISSIVE_MAP "OE_ENABLE_EMISSIVE_MAP"

#define oe_basecolor_map "oe_basecolor_map"
#define oe_normal_map "oe_normal_map"
#define oe_mr_map "oe_mr_map"
#define oe_ao_map "oe_ao_map"
#define oe_emissive_map "oe_emissive_map"



namespace osgEarth {

    namespace Util
    {
        //struct BasicPbrShaders : public osgEarth::Util::ShaderPackage
        //{
        //    virtual std::string materialInput() = 0;
        //    virtual std::string elevation() = 0;
        //    virtual std::string vert() = 0;
        //    virtual std::string frag() = 0;
        //    virtual std::string brdf() = 0;

        //    using SourceMap = std::map<std::string, std::string>;
        //    SourceMap _sourceMap;

        //};

        //struct ShadersGL3 : public BasicPbrShaders
        //{
        //    std::string materialInput() { return ""; }
        //    std::string elevation() { return ""; }
        //    std::string vert() {
        //        return "";
        //    }
        //    std::string frag() { return "";} 
        //    std::string brdf() { return ""; }
        //};
        //struct ShadersGL4 : public BasicPbrShaders
        //{
        //    std::string materialInput() { return ""; }
        //    std::string elevation() { return ""; }
        //    std::string vert() { return ""; }
        //    std::string frag() { return ""; }
        //    std::string brdf() { return ""; }
        //};

        //struct PbrShadersFactory
        //{
        //    PbrShadersFactory()
        //    {

        //    }
        //    static ShadersGL3 s_gl3;
        //    static ShadersGL4 s_gl4;
        //    static BasicPbrShaders& get(bool use_gl4) { return use_gl4 ? (BasicPbrShaders&)s_gl4 : (BasicPbrShaders&)s_gl3; }
        //};




        /**
         * Shader effect that performs simple Phong lighting.
         */
        class OE_MATERIAL_PULGIN PbrLightEffect : public osg::Referenced
        {
        public:
            /** constructs a new effect */
            PbrLightEffect();

            /** contructs a new effect and attaches it to a stateset. */
            PbrLightEffect(osg::StateSet* stateset);

            /** is it supported by the h/w? */
            bool supported() const { return _supported; }

        public:
            /** attach this effect to a stateset. */
            void attach(osg::StateSet* stateset);

            /** detach this effect from any attached statesets. */
            void detach();

            /** detach this effect from a stateset. */
            void detach(osg::StateSet* stateset);

        protected:
            virtual ~PbrLightEffect();

            
            typedef std::list< osg::observer_ptr<osg::StateSet> > StateSetList;
            

            bool _supported;
            StateSetList _statesets;
            //osg::ref_ptr<osg::Uniform> _lightingUniform;

            void init();

            using TextureCache = osgEarth::Mutexed<
                std::unordered_map<std::string, osg::ref_ptr<osg::Texture2D>> >;

            mutable TextureCache* _texCache;
           
        };

    }
} // namespace osgEarth::Util

#endif 