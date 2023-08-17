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

class ReflectionProbe:public osg::Object
{
public:
	ReflectionProbe(float near = 1.f, float far = 10.f, uint32_t resolution = 128);
	ReflectionProbe(const ReflectionProbe& teapot, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY) {}
	META_Object(osgIBLApp, ReflectionProbe);

	void createCubeCameras(float near=1.f, float far=100.f, uint32_t resolution = 128);
	void setPosition(osg::Vec3 pos);
	osg::Node* getNode();
	osg::Camera* createDebugHUDCamera();
	osg::Node* getCubeCameras() { return _camGp.get(); }
	void addReflectedGraph(osg::Node* graph);

	
	osg::ref_ptr<osg::Group> _camGp;
	osg::Vec3 _pos;
	osg::ref_ptr<osg::TextureCubeMap> _texture;
	osg::ref_ptr<osg::Node> _probe;
	osg::ref_ptr<osg::Node> _debugGeom;
};