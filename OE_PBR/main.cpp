
//#define NOMINMAX
#include<Windows.h>

//

#include<osg/GLExtensions>
#include <osgViewer/Viewer>
#include<osg/ShapeDrawable>
#include<osgGA/TrackballManipulator>
#include <osgEarth/Notify>
#include <osgEarth/EarthManipulator>
#include <osgEarth/ExampleResources>
#include <osgEarth/MapNode>
#include <osgEarth/Threading>
#include <osgEarth/ShaderGenerator>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgUtil/Optimizer>
#include <iostream>
#include<osgEarth/Registry>
#include<osgEarth/Capabilities>
#include <osgEarth/Metrics>

#include<osgEarth/GLUtils>
#include<osgEarth/Sky>
#include<osgEarth/PhongLightingEffect>
#include<osgEarth/VirtualProgram>

#include"PbrLightEffect.h"

#define LC "[viewer] "

#ifndef OSG_GL_FIXED_FUNCTION_AVAILABLE
#define OSG_GL_FIXED_FUNCTION_AVAILABLE
#endif // !1



using namespace osgEarth;
using namespace osgEarth::Util;
using NodePtr = osg::ref_ptr<osg::Node>;

int
usage(const char* name)
{
    OE_NOTICE
        << "\nUsage: " << name << " file.earth" << std::endl
        << MapNodeHelper().usage() << std::endl;

    return 0;
}


osg::ref_ptr<osg::Node> CreatePbrSphere()
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    //osg::ref_ptr<osg::Geode> geode = new osg::Geode();

    osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3f(0.0f, 0.0f, 0.0f), 2.0f));


    sd->setColor(osg::Vec4(1, 0, 0, 1));
    geode->addDrawable(sd);
    geode->getOrCreateStateSet()->setTextureAttribute(0, new osg::Texture2D(osgDB::readImageFile("D:/GitProject/FEngine/Assets/PbrBox/BoomBox_baseColor.png")), osg::StateAttribute::ON);

    auto* phong = new PhongLightingEffect();
    phong->attach(geode->getOrCreateStateSet());

   auto* vp = osgEarth::VirtualProgram::get(geode->getOrCreateStateSet());
   vp->setShaderLogging(true);

    return geode;

}

osg::Node* CreateLight(osg::StateSet* rootStateSet)
{
    osg::Group* lightGroup = new osg::Group;

    float modelSize = 5.0f;

    // create a spot light.
    osg::Light* myLight1 = new osg::Light;
    myLight1->setLightNum(0);
    myLight1->setPosition(osg::Vec4(1.0f, 1.0f, 0.0f, 0.0f));
    myLight1->setAmbient(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
    myLight1->setDiffuse(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
    myLight1->setSpotCutoff(20.0f);
    myLight1->setSpotExponent(50.0f);
    myLight1->setDirection(osg::Vec3(1.0f, 1.0f, -1.0f));

    osg::LightSource* lightS1 = new osg::LightSource;
    lightS1->setLight(myLight1);
    lightS1->setLocalStateSetModes(osg::StateAttribute::ON);

    lightS1->setStateSetModes(*rootStateSet, osg::StateAttribute::ON);
    lightGroup->addChild(lightS1);


    // create a local light.
    osg::Light* myLight2 = new osg::Light;
    myLight2->setLightNum(1);
    myLight2->setPosition(osg::Vec4(0.0, 0.0, 0.0, 1.0f));
    myLight2->setAmbient(osg::Vec4(0.0f, 1.0f, 1.0f, 1.0f));
    myLight2->setDiffuse(osg::Vec4(0.0f, 1.0f, 1.0f, 1.0f));
    myLight2->setConstantAttenuation(1.0f);
    myLight2->setLinearAttenuation(2.0f / modelSize);
    myLight2->setQuadraticAttenuation(2.0f / osg::square(modelSize));

    osg::LightSource* lightS2 = new osg::LightSource;
    lightS2->setLight(myLight2);
    lightS2->setLocalStateSetModes(osg::StateAttribute::ON);

    lightS2->setStateSetModes(*rootStateSet, osg::StateAttribute::ON);
    lightGroup->addChild(lightS2);
    return lightGroup;
}

int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    int versio = osg::getGLVersionNumber();
  /*  char* versionstring = (char*)glGetString(GL_VERSION);
    std::string ver(versionstring);
    if (osg::getGLVersionNumber() >= 3.0)
    {
        std::cout << "OpenGL version: " << ver << std::endl;
    }
    else {
        std::cout << "OpenGL version2: " << ver << std::endl;
    }*/
    
    

    //osgEarth::initialize();
    //GLUtils::enableGLDebugging();
    //VirtualProgram::enableGLDebugging();
    //osg::DisplaySettings::instance()->setGLContextVersion("3.0");
    //osg::DisplaySettings::instance()->setGLContextProfileMask(0x1);
    osgViewer::Viewer viewer(arguments);
    viewer.setReleaseContextAtEndOfFrameHint(false);
    viewer.getDatabasePager()->setUnrefImageDataAfterApplyPolicy(true, false);

    

    const int width(800), height(450);
    const std::string version("3.0");
    osg::ref_ptr< osg::GraphicsContext::Traits > traits = new osg::GraphicsContext::Traits();
    traits->x = 20; traits->y = 30;
    traits->width = width; traits->height = height;
    traits->windowDecoration = true;
    /*traits->glContextVersion = version;*/
    traits->glContextProfileMask = 0X1;
    traits->doubleBuffer = true;
    traits->readDISPLAY();
    traits->setUndefinedScreenDetailsToDefaultScreen();
    osg::ref_ptr< osg::GraphicsContext > gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    //gc->getState()->setCheckForGLErrors(osg::State::NEVER_CHECK_GL_ERRORS);
    osg::setNotifyLevel(osg::NotifySeverity::ALWAYS);
    if (!gc.valid())
    {
        osg::notify(osg::FATAL) << "Unable to create OpenGL v" << version << " context." << std::endl;
        return(1);
    }

    ////osgViewer::Viewer viewer;

    //// Create a Camera that uses the above OpenGL context.
    //osg::Camera* cam = viewer.getCamera();
    //cam->setGraphicsContext(gc.get());


    osgDB::Registry::instance()->getObjectWrapperManager()->findWrapper("osg::Image");


    auto group = new osg::Group();

    auto node = CreatePbrSphere();
     auto light = CreateLight(node->getOrCreateStateSet());
      group->addChild(light);

    group->addChild(node);
    viewer.setSceneData(group);

    viewer.setCameraManipulator(new osgGA::TrackballManipulator);
    osgUtil::Optimizer opt;
    opt.optimize(node, osgUtil::Optimizer::INDEX_MESH);
    ShaderGenerator gen;
    node->accept(gen);
    viewer.setUpViewInWindow(100, 100, 800, 600);
    viewer.realize();
    viewer.run();
    return 0;
}


//int main(int argc, char** argv)
//{
//    osgEarth::initialize();
//    osg::ArgumentParser arguments(&argc, argv);
//    GLUtils::enableGLDebugging();
//    VirtualProgram::enableGLDebugging();
//
//   /* osg::ref_ptr<osg::Node> root = CreatePbrSphere();
//    if (root == NULL)
//    {
//        osg::notify(osg::FATAL) << "Unable to load model from command line." << std::endl;
//        return(1);
//    }
//
//    osgUtil::Optimizer optimizer;
//    optimizer.optimize(root.get(), osgUtil::Optimizer::ALL_OPTIMIZATIONS | osgUtil::Optimizer::TESSELLATE_GEOMETRY);*/
//
//    //configureShaders(root->getOrCreateStateSet());
//
//   /* const int width(800), height(450);
//    const std::string version("3.3");
//    osg::ref_ptr< osg::GraphicsContext::Traits > traits = new osg::GraphicsContext::Traits();
//    traits->x = 20; traits->y = 30;
//    traits->width = width; traits->height = height;
//    traits->windowDecoration = true;
//    traits->glContextVersion = version;
//    traits->glContextProfileMask = 0X1;
//    traits->doubleBuffer = true;
//    traits->glContextVersion = version;
//    traits->readDISPLAY();
//    traits->setUndefinedScreenDetailsToDefaultScreen();
//    osg::ref_ptr< osg::GraphicsContext > gc = osg::GraphicsContext::createGraphicsContext(traits.get());
//    if (!gc.valid())
//    {
//        osg::notify(osg::FATAL) << "Unable to create OpenGL v" << version << " context." << std::endl;
//        return(1);
//    }*/
//
//    osgViewer::Viewer viewer;
//
//    // Create a Camera that uses the above OpenGL context.
//    osg::Camera* cam = viewer.getCamera();
//    //cam->setGraphicsContext(gc.get());
//    //// Must set perspective projection for fovy and aspect.
//    //cam->setProjectionMatrix(osg::Matrix::perspective(30., (double)width / (double)height, 1., 100.));
//    //// Unlike OpenGL, OSG viewport does *not* default to window dimensions.
//    //cam->setViewport(new osg::Viewport(0, 0, width, height));
//
//
//
//        // install our default manipulator (do this before calling load)
//    viewer.setCameraManipulator(new EarthManipulator(arguments));
//
//    // disable the small-feature culling
//    viewer.getCamera()->setSmallFeatureCullingPixelSize(-1.0f);
//
//    // load an earth file, and support all or our example command-line options
//    auto node = MapNodeHelper().load(arguments, &viewer);
//
//
//    viewer.setUpViewInWindow(100, 100, 800, 600);
//    auto& cap = osgEarth::Registry::instance()->capabilities();
//        std::cout << " " << cap.getGLSLVersion() << std::endl;
//    std::cout << " " << cap.isCoreProfile() << std::endl;
//    std::cout << " " << cap.getMaxGPUTextureUnits() << std::endl;
//    std::cout << " " << cap.getMaxGPUTextureUnits() << std::endl;
//    std::cout << " getMaxFastTextureSize " << cap.getMaxFastTextureSize() << std::endl;
//    std::cout << " supportsNVGL " << cap.supportsNVGL() << std::endl;
//    if (node.valid())
//    {
//        viewer.setSceneData(node);
//
//        if (!MapNode::get(node))
//        {
//            // not an earth file? Just view as a normal OSG node or image
//            viewer.setCameraManipulator(new osgGA::TrackballManipulator);
//            osgUtil::Optimizer opt;
//            opt.optimize(node, osgUtil::Optimizer::INDEX_MESH);
//            ShaderGenerator gen;
//            node->accept(gen);
//        }
//        Metrics::setEnabled(true);
//        return Metrics::run(viewer);
//    }
//    return 0;
//   /* viewer.setSceneData(root);
//
//    gc->getState()->setUseVertexAttributeAliasing(false);
//
//    return(viewer.run());*/
//}

//
//int
//main(int argc, char** argv)
//{
//    osgEarth::initialize();
//
//    osg::ArgumentParser arguments(&argc, argv);
//
//    //arguments.read("D:\\LocalSDK\\osgearth\\tests\\annotation.earth");
//    // help?
//    if (arguments.read("--help"))
//        return usage(argv[0]);
//
//    osg::DisplaySettings::instance()->setGLContextVersion("4.0");
//    osg::DisplaySettings::instance()->setGLContextProfileMask(0x1);
//
//   /* GLUtils::enableGLDebugging();
//    VirtualProgram::enableGLDebugging();*/
//
//    const int width(800), height(450);
//    const std::string version("3.3");
//    osg::ref_ptr< osg::GraphicsContext::Traits > traits = new osg::GraphicsContext::Traits();
//    traits->x = 20; traits->y = 30;
//    traits->width = width; traits->height = height;
//    traits->windowDecoration = true;
//    traits->glContextVersion = version;
//    traits->glContextProfileMask = 0X1;
//    traits->doubleBuffer = true;
//    traits->glContextVersion = version;
//    traits->readDISPLAY();
//    traits->setUndefinedScreenDetailsToDefaultScreen();
//    osg::ref_ptr< osg::GraphicsContext > gc = osg::GraphicsContext::createGraphicsContext(traits.get());
//    if (!gc.valid())
//    {
//        osg::notify(osg::FATAL) << "Unable to create OpenGL v" << version << " context." << std::endl;
//        return(1);
//    }
//    // create a viewer:
//    osgViewer::Viewer viewer(arguments);
//    // This is normally called by Viewer::run but we are running our frame loop manually so we need to call it here.
//    viewer.setReleaseContextAtEndOfFrameHint(false);
//
//    // Tell the database pager to not modify the unref settings
//    viewer.getDatabasePager()->setUnrefImageDataAfterApplyPolicy(true, false);
//
//    // thread-safe initialization of the OSG wrapper manager. Calling this here
//    // prevents the "unsupported wrapper" messages from OSG
//    osgDB::Registry::instance()->getObjectWrapperManager()->findWrapper("osg::Image");
//
//
//
//
//        // Create a Camera that uses the above OpenGL context.
//    osg::Camera* cam = viewer.getCamera();
//    cam->setGraphicsContext(gc.get());
//    // Must set perspective projection for fovy and aspect.
//    //cam->setProjectionMatrix(osg::Matrix::perspective(30., (double)width / (double)height, 1., 100.));
//    //// Unlike OpenGL, OSG viewport does *not* default to window dimensions.
//    //cam->setViewport(new osg::Viewport(0, 0, width, height));
//
//
//    // install our default manipulator (do this before calling load)
//    viewer.setCameraManipulator(new EarthManipulator(arguments));
//
//    // disable the small-feature culling
//    viewer.getCamera()->setSmallFeatureCullingPixelSize(-1.0f);
//
//    // load an earth file, and support all or our example command-line options
//    auto node = MapNodeHelper().load(arguments, &viewer);
//
//   /* osgEarth::SkyNode* sky = osgEarth::Util::SkyNode::create(opt);
//    sky->attach(&viewer);
//    sky->setAtmosphereVisible(true);
//    sky->addChild(node);*/
//
//    viewer.setUpViewInWindow(100, 100, 800, 600);
//    if (node.valid())
//    {
//        viewer.setSceneData(node);
//
//        if (!MapNode::get(node))
//        {
//            // not an earth file? Just view as a normal OSG node or image
//            viewer.setCameraManipulator(new osgGA::TrackballManipulator);
//            osgUtil::Optimizer opt;
//            opt.optimize(node, osgUtil::Optimizer::INDEX_MESH);
//            ShaderGenerator gen;
//            node->accept(gen);
//        }
//        Metrics::setEnabled(true);
//        return Metrics::run(viewer);
//    }
//
//    return usage(argv[0]);
//}