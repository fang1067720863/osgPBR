
#define NOMINMAX
#include <Windows.h>

#include "TestUI.h"

#include <osg/GLExtensions>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>
#include <osgEarth/EarthManipulator>
#include <osgEarth/ExampleResources>
#include <osgEarth/MapNode>
#include <osgEarth/Notify>
#include <osgEarth/ShaderGenerator>
#include <osgEarth/Threading>
#include <osgGA/TrackballManipulator>
#include <osgManipulator/TranslateAxisDragger>
#include <osgShadow/ConvexPolyhedron>
#include <osgViewer/Viewer>

#include <iostream>
#include <osgEarth/Capabilities>
#include <osgEarth/Lighting>
#include <osgEarth/Metrics>
#include <osgEarth/Registry>
#include <osgUtil/Optimizer>

#include <osg/Texture2D>
#include <osg/Texture>
#include <osg/TextureCubeMap>
#include <osgEarth/GLUtils>
#include <osgEarth/PhongLightingEffect>
#include <osgEarth/Sky>
#include <osgEarth/VirtualProgram>

#include "GLTFV2Reader.h"
#include "CreateHelper.h"
#include "FlyCameraManipulator.h"

// #define LC "[viewer] "

const std::string PROJECT_PATH = "..//OE_PBR//";

template <typename T>
struct CB : public osg::NodeCallback
{
	using F = std::function<void(T*, osg::NodeVisitor*)>;
	F _func;
	CB(F func)
	    : _func(func)
	{
	}
	void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		_func(static_cast<T*>(node), nv);
	}
};

using namespace osgEarth;
using namespace osgEarth::Util;
using NodePtr = osg::ref_ptr<osg::Node>;

int usage(const char* name)
{
	OE_NOTICE
	    << "\nUsage: " << name << " file.earth" << std::endl
	    << MapNodeHelper().usage() << std::endl;

	return 0;
}

class CullCallback : public osg::NodeCallback
{
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		std::cout << "cull callback - pre traverse" << node << std::endl;
		traverse(node, nv);
		std::cout << "cull callback - post traverse" << node << std::endl;
	}
};


void SetupSceneGraph(osgViewer::Viewer& viewer)
{
	osg::ref_ptr<osgDB::Options> modelDB = new osgDB::Options("model");
	modelDB->setDatabasePath(PROJECT_PATH + "Asset");

	auto iblDB = new osgDB::Options("IBL");
	iblDB->setDatabasePath(PROJECT_PATH + "Asset//IBL");

	osg::ref_ptr<osgDB::Options> shaderDB = new osgDB::Options();
	shaderDB->setName("osgEarthShader");
	shaderDB->setDatabasePath(PROJECT_PATH + "Asset//Shader");
	osgDB::Registry::instance()->getObjectWrapperManager()->findWrapper("osg::Image");


	auto baseGraph = new osg::Group();

	// init env light
	EnvLightEffect::instance()->setEnvMapAtlas({ "pisaHDR\\diffuse.dds", "pisaHDR\\specular.dds", "pisaHDR\\env.dds" }, iblDB);
	EnvLightEffect::instance()->setEnable(true);

	// init models
	GLTFReaderV2 reader;
	auto gltfModel = reader.read("Dragon\\DragonAttenuation.gltf", false, modelDB);
	// Helmet\\DamagedHelmet   BoomBox\\BoomBox Sponza\\Sponza  Sheen\\SheenChair.glb  Dragon\\DragonAttenuation
	auto gltfNode = gltfModel.getNode();

	// init material spheres
	auto materialSpheres = createMaterialSpheres(2);
	baseGraph->addChild(materialSpheres);
	
	// init skybox
	baseGraph->addChild(createSkyBox());

	//init lights
	osg::Light* lightState = new osg::Light;
	auto light = CreateLight(lightState, baseGraph);
	baseGraph->addChild(light);

	auto func = [&](osg::MatrixTransform* node, osg::NodeVisitor* nv)
	{
		auto matrix = node->getMatrix();

		node->setMatrix(matrix * osg::Matrix(osg::Quat(0.01, osg::Vec3(0.0, 0.0, 1.0))));
	};

	osg::Group* sceneData = new osg::Group;
	sceneData->addChild(baseGraph);

	auto nv = new GenerateEnvLightUniforms();
	baseGraph->accept(*nv);
	auto gpNV = new GenerateProbeUniforms();
	baseGraph->accept(*gpNV);

	// init transparent pass
	TransparentCamera::Ptr tCam = new TransparentCamera();
	tCam->setView(&viewer);
	tCam->setGraph(baseGraph);

	sceneData->addChild(tCam);
	viewer.setSceneData(sceneData);
	
	// init gui
	GUI::ApplicationGUI* gui = new GUI::ApplicationGUI(true);
	auto materialPanel = new MaterialGUI();
	gui->add("Demo", materialPanel);
	gui->add("Demo2", new LightGUI(lightState));
	gui->add("Demo3", new IndirectLightGUI());

	viewer.getEventHandlers().push_front(gui);

	auto findCallback = [materialPanel](osg::MatrixTransform* node)
	{
		node->getName();
		materialPanel->setNode(node);
		return true;
	};
	viewer.addEventHandler(new RayPicker(&viewer, findCallback));
}


int main(int argc, char** argv)
{

	osg::ArgumentParser arguments(&argc, argv);
	int versio = osg::getGLVersionNumber();
	arguments.getApplicationUsage()->setApplicationName("osgEarth PBR Material");
	osgViewer::Viewer viewer(arguments);
	viewer.setReleaseContextAtEndOfFrameHint(false);
	viewer.getDatabasePager()->setUnrefImageDataAfterApplyPolicy(true, false);
	auto name = arguments.getApplicationUsage()->getApplicationName();

	const int width(800), height(450);
	const std::string version("3.0");
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits();
	traits->x = 20;
	traits->y = 30;
	traits->width = width;
	traits->height = height;
	traits->windowDecoration = true;
	traits->glContextProfileMask = 0X1;
	traits->doubleBuffer = true;
	traits->readDISPLAY();
	traits->setUndefinedScreenDetailsToDefaultScreen();
	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
	osg::setNotifyLevel(osg::NotifySeverity::ALWAYS);
	if (!gc.valid())
	{
		osg::notify(osg::FATAL) << "Unable to create OpenGL v" << version << " context." << std::endl;
		return (1);
	}	

	viewer.setReleaseContextAtEndOfFrameHint(false);
	viewer.setRealizeOperation(new GUI::ApplicationGUI::RealizeOperation);

	SetupSceneGraph(viewer);

	viewer.setCameraManipulator(new osgGA::UnityCameraManipulator);
	viewer.setUpViewInWindow(100, 100, 800, 600);
	osgViewer::Viewer::Windows windows;
	viewer.getWindows(windows);
	//windows.front()->setCursor(osgViewer::GraphicsWindow::MouseCursor::HandCursor);
	viewer.realize();
	
	Metrics::setEnabled(true);
	Metrics::setGPUProfilingEnabled(true);
	return Metrics::run(viewer);
	/*  viewer.run();*/
	/*return 0;*/
}
