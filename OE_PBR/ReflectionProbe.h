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
	void setPosition(osg::Vec3d pos);
	osg::ref_ptr<osg::TextureCubeMap> getCubeMap() { return _texture; }
	osg::Vec3d getPos() { return _pos; }
	osg::Node* getNode();
	osg::Camera* createDebugHUDCamera();
	osg::Node* getCubeCameras() { return _camGp.get(); }
	void addReflectedGraph(osg::Node* graph);
	void setCullMask(osg::Node::NodeMask mask);
	void installProbeCallback();

	
	osg::ref_ptr<osg::Group> _camGp;
	osg::Vec3d _pos;
	osg::ref_ptr<osg::TextureCubeMap> _texture;
	osg::ref_ptr<osg::Node> _probe;
	osg::ref_ptr<osg::Node> _debugGeom;
	
};


class ProbeManager :public osg::Object
{
public:
	META_Object(osgEarth, ProbeManager)
	static osg::ref_ptr<ProbeManager>& instance();
	void addProbe(osg::ref_ptr<ReflectionProbe> probe);
	osg::ref_ptr<ReflectionProbe> createProbe();
	osg::ref_ptr<ReflectionProbe> getProbe(int index) const;

	void setTransmissionMap(osg::Texture2D* tex) { transmissionMap = tex; }
	osg::Texture2D* getTransmissionMap() { return transmissionMap.get(); }
	bool empty() const;
protected:
	ProbeManager(){}
	ProbeManager(const ProbeManager& rhs, const osg::CopyOp& copyop = osg::CopyOp::DEEP_COPY_ALL) {}
	std::vector< osg::ref_ptr<ReflectionProbe>> _probes;
	osg::ref_ptr< osg::Texture2D> transmissionMap;
	
};
class  GenerateProbeUniforms : public osg::NodeVisitor
{
public:
	GenerateProbeUniforms();

public: // osg::NodeVisitor
	virtual void apply(osg::Node& node);

protected:
	std::set<osg::StateSet*> _statesets;

	std::function<void(osg::StateAttribute*, osg::NodeVisitor*)> _probeCallback;

	template<typename T> bool alreadyInstalled(osg::Callback* cb) const {
		return !cb ? false : dynamic_cast<T*>(cb) != 0L ? true : alreadyInstalled<T>(cb->getNestedCallback());
	}
};

class TransparentCamera : public osg::Node
{
	
public:
	using Ptr = osg::ref_ptr<TransparentCamera>;
    //! Construct a new picker
	TransparentCamera();

    //! View within which to pick
    void setView(osgViewer::View* view);
    osgViewer::View* getView() const { return _view.get(); }

    //! Scene graph from which to pick
    void setGraph(osg::Node*);
    osg::Node* getGraph() const { return _graph.get(); }

public: // for debugging

    /** For debugging only - creates (if nec.) and returns a texture that captures
        the RTT image so you can display it. */
	osg::Texture2D* getOrCreateTexture();

public: // osg::Node
    void traverse(osg::NodeVisitor&) override;

protected:

    /** dtor */
    virtual ~TransparentCamera();

    osg::observer_ptr<osgViewer::View> _view;
    osg::ref_ptr<osg::Image> _pickImage;
    osg::ref_ptr<osg::Camera> _rtt;
    osg::ref_ptr<osg::Node> _graph;
    osg::ref_ptr<osg::Texture2D> _debugTex;


    void setupRTT(osgViewer::View*);

    int _rttSize; // size of the RTT image (pixels per side)
    int _buffer; // buffer around pick point to check (pixels)

};