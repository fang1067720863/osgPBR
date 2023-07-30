#pragma once

//author:1067720863@qq.com
//create date:2023/04
//decription: Cook_BRDF Light

#include<Windows.h>

#include <osg/StateSet>
#include<osg/Texture2D>
#include <osg/Uniform>
#include <osg/observer_ptr>


#include <osgEarth/Common>
#include<osgEarth/Threading>
#include<osgEarth/URI>
#include <osgEarth/ShaderLoader>
#include<osgEarth/VirtualProgram>
#include <set>
#include <osg/Node>
#include <osg/NodeVisitor>
#include <osg/LightSource>
#include <osg/Light>
#include<osg/Texture>
#include<osg/Vec4>
#include<osg/Camera>
#include<osg/State>
#include<osg/StateAttribute>
#include <osg/TextureCubeMap>
#include <osgEarth/Common>
#include <osgEarth/Threading>
#include"Export.h"

#include <osgViewer/Viewer>
#include<osg/Group>
using namespace osgEarth;

class EnvLightEffect;
class EnvLightSource : public osg::Group
{
public:

    EnvLightSource(){}

    /** Copy constructor using CopyOp to manage deep vs shallow copy. */
    EnvLightSource(const EnvLightSource& ls,
        const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY) :
        Group(ls, copyop) {}

    META_Node(osg, EnvLightSource);

    static osg::ref_ptr<EnvLightEffect>& getEnvLightEffect();
    enum ReferenceFrame
    {
        RELATIVE_RF,
        ABSOLUTE_RF
    };
protected:
    virtual ~EnvLightSource();

};


class  EnvLightGL3UniformGenerator : public osg::NodeCallback
{
public:
    EnvLightGL3UniformGenerator();

public:

    bool run(osg::Object* obj, osg::Object* data);

public: // osg::Object

    void resizeGLBufferObjects(unsigned maxSize);
    void releaseGLObjects(osg::State* state) const;

    mutable std::vector<osg::ref_ptr<osg::StateSet> > _statesets;
    mutable Threading::Mutex _statesetsMutex;
};

class EnvLightEffect:public osg::Object
{
    friend class EnvLightSource;
    friend class EnvLightGL3UniformGenerator;

    struct EnvMapConfig
    {
        std::string _diffuseMap;
        std::string _specularMap;
        std::string _envMap;
    };
    using Functor = std::function<void(void)>;
public:
    META_Object(osgEarth, EnvLightEffect)

    static osg::ref_ptr<EnvLightEffect>& instance();
   
    bool enabled() const { return _enable; }
    bool useCubeUV() const { return _useCubeUV; }
    float lightIntensity() const { return _lightIntensity; }
    float maxReflectionLOD() const { return _maxReflectionLOD; }
    void setEnable(bool enable);
    void setLightIntensity(float i) { _lightIntensity = i; }
    void setEnvMapAtlas(const EnvMapConfig& config, osgDB::Options* opts = nullptr);
    void addReplaceCallback(Functor&& f) { replaceCallbacks.push_back(std::move(f)); }
   
protected:
    EnvLightEffect();
    EnvLightEffect(const std::string& envCubeFile);
    void _ResetEnvMapAtlas();

    osg::Texture* getEnvCubeMap() { return _enable ? envCubeMap.get() : nullptr; }
    osg::Texture* getIrridianceMap() { return _enable ? irridianceMap.get() : nullptr; }
    osg::Texture* getPrefilterMap() { return _enable ? prefilterMap.get() : nullptr; }
    osg::Texture* getBrdfLUTMap() { return _enable ? brdfLUTMap.get() : nullptr; }

private:
    EnvLightEffect(const EnvLightEffect& rhs, const osg::CopyOp& copyop = osg::CopyOp::DEEP_COPY_ALL) {  }
    bool _enable;
    bool _texturesReady;
    bool _useCubeUV{ false };
    osg::ref_ptr<osg::Texture> envCubeMap;
    osg::ref_ptr<osg::Texture> irridianceMap;
    osg::ref_ptr<osg::Texture> prefilterMap;
    osg::ref_ptr<osg::Texture2D> brdfLUTMap;
    osg::ref_ptr<osgDB::Options> _options{ 0 };

   
    EnvMapConfig _config;
    float _lightIntensity{ 0.0f };
    float _maxReflectionLOD{ 1.0f };
    std::vector<Functor> replaceCallbacks;
    
};

