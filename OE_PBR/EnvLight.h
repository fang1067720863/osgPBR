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
using namespace osgEarth;

class EnvLightEffect:public osg::Object
{
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
    void setEnable(bool enable) {
        _enable = enable;
        if (enable && !_texturesReady) InitEnvMapAtlas();
    }
   
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
};

