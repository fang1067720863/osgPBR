#pragma once
#pragma once

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
        struct BasicPbrShaders : public osgEarth::Util::ShaderPackage
        {
            virtual std::string materialInput() = 0;
            virtual std::string elevation() = 0;
            virtual std::string vert() = 0;
            virtual std::string frag() = 0;
            virtual std::string brdf() = 0;

            using SourceMap = std::map<std::string, std::string>;
            SourceMap _sourceMap;

            //std::string load_raw_source(
            //    const std::string& filename,
            //    const osgDB::Options* dbOptions = 0L)
            //{
            //    std::string output;

            //    SourceMap::const_iterator iter = _sourceMap.find(filename);
            //    if (iter != _sourceMap.end())
            //    {
            //        output = iter->second;
            //    }

            //    if (!filename.empty())
            //    {
            //        URIContext context(dbOptions);
            //        URI uri(filename, context);

            //        // searches OSG_FILE_PATH
            //        std::string path = osgDB::findDataFile(uri.full(), dbOptions);
            //        if (!path.empty())
            //        {
            //            std::string externalSource = URI(path, context).getString(dbOptions);
            //            if (!externalSource.empty())
            //            {
            //                OE_DEBUG << "Loaded external shader " << filename << " from " << path << "\n";
            //                output = externalSource;
            //            }
            //        }
            //    }

            //    if (output.empty())
            //    {
            //        OE_WARN << "No shader source found for \"" << filename << "\"" << std::endl;
            //    }

            //    return output;
            //}
        };

        struct ShadersGL3 : public BasicPbrShaders
        {
            std::string materialInput() { return ""; }
            std::string elevation() { return ""; }
            std::string vert() {
                return "";
            } //return load_raw_source(osgEarth::getAbsolutePath("..//OE_PBR/Shader//vert.glsl")); }
            std::string frag() { return "";} //return load_raw_source(osgEarth::getAbsolutePath("..//OE_PBR/Shader//.glsl")); 
            std::string brdf() { return ""; }//return load_raw_source(osgEarth::getAbsolutePath("..//OE_PBR/Shader//BRDF.glsl")); }
        };
        struct ShadersGL4 : public BasicPbrShaders
        {
            std::string materialInput() { return ""; }
            std::string elevation() { return ""; }
            std::string vert() { return ""; }
            std::string frag() { return ""; }
            std::string brdf() { return ""; }
        };

        struct PbrShadersFactory
        {
            PbrShadersFactory()
            {

            }
            static ShadersGL3 s_gl3;
            static ShadersGL4 s_gl4;
            static BasicPbrShaders& get(bool use_gl4) { return use_gl4 ? (BasicPbrShaders&)s_gl4 : (BasicPbrShaders&)s_gl3; }
        };



        struct PbrMaterial
        {
            osg::Vec4f  baseColorFactor{ 1.0f,1.0f,1.0f,1.0f };
            osg::Vec4f  emissiveFactor{ 0.1f,0.1f,0.1f,1.0f };
            float metallicFactor;
            float roughnessFactor;
            float alphaMask;
            float alphaMaskCutoff;
            float aoStrength;
        };

        struct MaterialURI
        {
            std::string baseColorMap;
            std::string normalMap;
            std::string emissiveMap;
            std::string metalRoughnessMap;
            std::string occulusionMap;
        };

        /**
         * Shader effect that performs simple Phong lighting.
         */
        class PbrLightEffect : public osg::Referenced
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
            void enableTextureUnit(osg::StateSet* stateset, unsigned int texUnit, const osgEarth::URI& uri, const std::string& define, const std::string& texName);

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

#endif // OSGEARTH_PHONG_LIGHTING_EFFECT_H