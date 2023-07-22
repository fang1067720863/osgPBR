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

    /** Set the light sources's ReferenceFrame, either to be relative to its
      * parent reference frame, or relative to an absolute coordinate
      * frame. RELATIVE_RF is the default.
      * Note: setting the ReferenceFrame to be ABSOLUTE_RF will
      * also set the CullingActive flag on the light source, and hence all
      * of its parents, to false, thereby disabling culling of it and
      * all its parents.  This is necessary to prevent inappropriate
      * culling, but may impact cull times if the absolute light source is
      * deep in the scene graph.  It is therefore recommended to only use
      * absolute light source at the top of the scene.
    */
    //void setReferenceFrame(ReferenceFrame rf);

    //ReferenceFrame getReferenceFrame() const { return _referenceFrame; }

    ///** Set the attached light. */
    //void setLight(Light* light);

    ///** Get the attached light. */
    //inline Light* getLight() { return _light.get(); }

    ///** Get the const attached light. */
    //inline const Light* getLight() const { return _light.get(); }

    ///** Set the GLModes on StateSet associated with the LightSource. */
    //void setStateSetModes(StateSet&, StateAttribute::GLModeValue) const;

    ///** Set up the local StateSet. */
    //void setLocalStateSetModes(StateAttribute::GLModeValue value = StateAttribute::ON);

    ///** Set whether to use a mutex to ensure ref() and unref() are thread safe.*/
    //virtual void setThreadSafeRefUnref(bool threadSafe);

    //virtual BoundingSphere computeBound() const;

protected:

    virtual ~EnvLightSource();

    //StateAttribute::GLModeValue     _value;
    //ref_ptr<Light>                  _light;

  /*  ReferenceFrame                  _referenceFrame;*/
};


class  EnvLightGL3UniformGenerator : public osg::NodeCallback
{
public:
    EnvLightGL3UniformGenerator();

    /**
     * Creates and installs Uniforms on the stateset for the Light components
     * of the Light that are non-positional (everything but the position and direction)
     */
  /*  void generateNonPositionalData(osg::StateSet* ss, osg::Light* light);*/

public: // osg::NodeCallback

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
public:
    META_Object(osgEarth, EnvLightEffect)

    /** Maintain a DisplaySettings singleton for objects to query at runtime.*/
    static osg::ref_ptr<EnvLightEffect>& instance();
    osg::Texture* getEnvCubeMap() { return _enable ? envCubeMap.get() : nullptr; }
    osg::Texture* getIrridianceMap() { return _enable ? irridianceMap.get() : nullptr; }
    osg::Texture* getPrefilterMap() { return _enable ? prefilterMap.get() : nullptr; }
    osg::Texture* getBrdfLUTMap() { return _enable ? brdfLUTMap.get() : nullptr; }

    bool enabled() const { return _enable; }
    bool useCubeUV() const { return _useCubeUV; }
    float lightIntensity() const { return _lightIntensity; }
    void setEnable(bool enable) {
        _enable = enable;
        if (enable && !_texturesReady) InitEnvMapAtlas();
        _lightIntensity = 0.5;
    }
    void setLightIntensity(float i) { _lightIntensity = i; }
   
protected:
    EnvLightEffect() :_enable(false), _texturesReady(false){}
    EnvLightEffect(const std::string& envCubeFile):_enable(false), _texturesReady(false)
    {


    }
    

    void changeEnvMap() {}
    void InitEnvMapAtlas();
private:
    EnvLightEffect(const EnvLightEffect& rhs, const osg::CopyOp& copyop = osg::CopyOp::DEEP_COPY_ALL) {  }
    bool _enable;
    bool _texturesReady;
    bool _useCubeUV{ false };
    osg::ref_ptr<osg::Texture> envCubeMap;
    osg::ref_ptr<osg::Texture> irridianceMap;
    osg::ref_ptr<osg::Texture> prefilterMap;
    osg::ref_ptr<osg::Texture2D> brdfLUTMap;
    float _lightIntensity{ 0.0f };
};

