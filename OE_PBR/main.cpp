
//#define NOMINMAX
#include<Windows.h>

#include"TestUI.h"

#include<osg/GLExtensions>
#include <osgViewer/Viewer>
#include<osg/ShapeDrawable>
#include<osgGA/TrackballManipulator>

#include<osg/MatrixTransform>
#include <osgEarth/Notify>
#include <osgEarth/EarthManipulator>
#include <osgEarth/ExampleResources>
#include <osgEarth/MapNode>
#include <osgEarth/Threading>
#include <osgEarth/ShaderGenerator>
#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>
#include <iostream>
#include<osgEarth/Registry>
#include<osgEarth/Capabilities>
#include <osgEarth/Metrics>
#include<osgEarth/Lighting>

#include<osgEarth/GLUtils>
#include<osgEarth/Sky>
#include<osgEarth/PhongLightingEffect>
#include<osgEarth/VirtualProgram>
#include<osg/TextureCubeMap>


#include"PbrLightEffect.h"
#include"PbrMaterial.h"
#include"GLTFV2Reader.h"
#include"FlyCameraManipulator.h"
#include"EnvLight.h"
#include"IBLBaker.h"
#include"CubeToQuad.h"
#include"RayPicker.h"
#include"SkyBox.h"

//#define LC "[viewer] "

template<typename T>
struct CB : public osg::NodeCallback
{
    using F = std::function<void(T*, osg::NodeVisitor*)>;
    F _func;
    CB(F func) : _func(func) { }
    void operator()(osg::Node* node, osg::NodeVisitor* nv) {
        _func(static_cast<T*>(node), nv);
    }
};

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

class CullCallback : public osg::NodeCallback
{
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        std::cout << "cull callback - pre traverse" << node << std::endl;
        traverse(node, nv);
        std::cout << "cull callback - post traverse" << node << std::endl;
    }
};

struct DrawCallback : public osg::Drawable::DrawCallback
{

    DrawCallback() :
        _firstTime(true) {}

    virtual void drawImplementation(osg::RenderInfo& renderInfo, const osg::Drawable* drawable) const
    {
        osg::State& state = *renderInfo.getState();

        //ZoneNamedN(___tracy_scoped_zone, "CustomDrawCallback", true);
        //OE_PROFILING_ZONE_NAMED("CustomDrawCallback");

        drawable->drawImplementation(renderInfo);
    }

    mutable bool _firstTime;
};
osg::ref_ptr<osg::Node> CreatePbrSphere()
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();

    osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3f(0.0f, 0.0f, 0.0f), 2.0f));

    geode->addDrawable(sd);
   
    bool usePhong = false;
    bool usePBR = true;
    if (usePhong)
    {
      
        auto* phong = new PhongLightingEffect();
        phong->attach(geode->getOrCreateStateSet());

        osg::ref_ptr<osg::Material> m = new osgEarth::MaterialGL3();
        m->setAmbient(m->FRONT_AND_BACK, osg::Vec4(.5, .5, .5, 1));
        m->setDiffuse(m->FRONT_AND_BACK, osg::Vec4(1, 1, 1, 1));
        m->setSpecular(m->FRONT_AND_BACK, osg::Vec4(0.2, 0.2, 0.2, 1));
        m->setEmission(m->FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
        m->setShininess(m->FRONT_AND_BACK, 40.0);
        geode->getOrCreateStateSet()->setAttributeAndModes(m, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        MaterialCallback().operator()(m, 0L);
       

    }
    else if (usePBR) {
       
        osg::ref_ptr<osgEarth::StandardPBRMaterial> m = new osgEarth::StandardPBRMaterial();
        m->setName("PBR_MATERIAL");

        m->setBaseColorFactor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
        m->setEmissiveFactor(osg::Vec3f(1.0f, 1.0f, 1.0f));
        m->setMetallicFactor(0.56f);
        m->setRoughnessFactor(0.22f);
        m->setAoStrength(0.15f);

        std::string dir = "D:/GitProject/FEngine/Assets/PbrBox/", format = ".png";
        m->setTextureAttribute(osgEarth::StandardPBRMaterial::NormalMap, dir + "BoomBox_normal" + format);
        m->setTextureAttribute(osgEarth::StandardPBRMaterial::MetalRoughenssMap, dir + "BoomBox_roughnessMetallic" + format);
        m->setTextureAttribute(osgEarth::StandardPBRMaterial::OcclusionMap, dir + "BoomBox_occlusion" + format);
        m->setTextureAttribute(osgEarth::StandardPBRMaterial::EmissiveMap, dir + "BoomBox_emissive" + format);
        m->setTextureAttribute(osgEarth::StandardPBRMaterial::BaseColorMap, dir + "BoomBox_baseColor" + format);

        geode->getOrCreateStateSet()->setAttributeAndModes(m, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);


        m->setReceiveEnvLight(true);



        PBRMaterialCallback().operator()(m, 0L);

        auto* pbr = new PbrLightEffect();
        pbr->attach(geode->getOrCreateStateSet());

    }
   
    return geode;

}

osg::ref_ptr<osg::Node> CreateExtensionedMaterialSphere()
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();

    osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3f(0.0f, 0.0f, 0.0f), 2.0f));

    geode->addDrawable(sd);
    osg::ref_ptr<osgEarth::ExtensionedMaterial> m = new osgEarth::ExtensionedMaterial();
    m->setName("PBR_MATERIAL");

    m->setBaseColorFactor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    m->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
    m->setMetallicFactor(1.00f);
    m->setRoughnessFactor(0.88f);
    m->setAoStrength(0.15f);

    std::string dir = "C:/Users/10677/source/repos/OE_PBR/OE_PBR/Asset/Material/metal/", format = ".png";
    m->addTextureAttribute("metalMap", dir + "metal" + format, "OE_ENABLE_Metal_MAP");
    m->addTextureAttribute("roughnessMap", dir + "rough" + format, "OE_ENABLE_Roughness_MAP");

    m->setTextureAttribute(osgEarth::StandardPBRMaterial::NormalMap, dir + "normal" + format);

    m->setMaterialFile("materials/metalroughness.glsl");


    geode->getOrCreateStateSet()->setAttributeAndModes(m, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);


    m->setReceiveEnvLight(false);

    ExtensionedMaterialCallback().operator()(m, 0L);

    auto* pbr = new PbrLightEffect();
    pbr->attach(geode->getOrCreateStateSet());

    auto* vp = osgEarth::VirtualProgram::get(geode->getOrCreateStateSet());
    vp->setShaderLogging(true);
   
    return geode;
}
osg::Node* CreateLight(osg::Light * myLight1)
{
    osg::Group* lightGroup = new osg::Group;

    float modelSize = 5.0f;

    myLight1->setLightNum(0);
    myLight1->setPosition(osg::Vec4(1.0f, 1.0f, 0.0f, 0.0f));
    myLight1->setAmbient(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    myLight1->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    myLight1->setSpotCutoff(20.0f);
    myLight1->setSpotExponent(0.5f);
    myLight1->setDirection(osg::Vec3(-0.382353f, -0.254902f, -0.382353f));

    osg::LightSource* lightS1 = new osg::LightSource;
    lightS1->setLight(myLight1);
    lightS1->setLocalStateSetModes(osg::StateAttribute::ON);

   /* lightS1->setStateSetModes(*rootStateSet, osg::StateAttribute::ON);*/
    lightGroup->addChild(lightS1);
#ifdef NDEBUG
    auto cb = new LightSourceGL3UniformGenerator();
    lightS1->addCullCallback(cb);
#endif 

    EnvLightSource* els = new EnvLightSource;
    els->addCullCallback(new EnvLightGL3UniformGenerator());
    lightGroup->addChild(els);
    return lightGroup;
}

std::vector<osg::ref_ptr<StandardPBRMaterial>> createNoTexMaterials()
{
    std::vector<osg::ref_ptr<StandardPBRMaterial>> result;

    for (int i = 0; i <9; i++)
    {
        for (int j = 0; j <9; j++)
        {
            osg::ref_ptr<osgEarth::StandardPBRMaterial> m = new osgEarth::StandardPBRMaterial();

            m->setBaseColorFactor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
            m->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
            m->setMetallicFactor(0.11f * i+0.01);
            m->setRoughnessFactor(0.11f * j+0.01);
            m->setAoStrength(0.15f);
            m->setReceiveEnvLight(true);
            result.push_back(m);
        }
       
    }
    
    return result;
}
std::vector<osg::ref_ptr<ExtensionedMaterial>> createMaterials()
{
    std::vector<osg::ref_ptr<ExtensionedMaterial>> result;

    auto absolutePath = osgEarth::getAbsolutePath("Asset/Material");
    osg::ref_ptr<osgDB::Options> dbo = new osgDB::Options();
    if (osgDB::fileExists(absolutePath))
    {
        dbo->setDatabasePath(absolutePath);
    }



    

    osg::ref_ptr<osgEarth::ExtensionedMaterial> grass = new osgEarth::ExtensionedMaterial();
    grass->setName("grass");
    grass->setDataBaseOption(dbo);
   
    grass->setBaseColorFactor(osg::Vec4f(0.5f, 0.5f, 0.5f, 0.5f));
    grass->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
    grass->setMetallicFactor(0.29f);
    grass->setRoughnessFactor(0.81f);
    grass->setAoStrength(0.15f);
    grass->setTextureAttribute(osgEarth::StandardPBRMaterial::NormalMap, "grass/normal.png");
   // grass->setTextureAttribute(osgEarth::StandardPBRMaterial::OcclusionMap, "grass/ao.png");
    grass->setTextureAttribute(osgEarth::StandardPBRMaterial::BaseColorMap, "grass/albedo.png");
    grass->setReceiveEnvLight(true);
    
    grass->addTextureAttribute("metalMap", "grass/metallic.png", "OE_ENABLE_Metal_MAP");
    grass->addTextureAttribute("roughnessMap", "grass/roughness.png", "OE_ENABLE_Roughness_MAP");
    grass->setMaterialFile("materials/metalroughness.glsl");
    result.push_back(grass);

    osg::ref_ptr<osgEarth::ExtensionedMaterial> m = new osgEarth::ExtensionedMaterial();
    m->setName("metal");
    m->setDataBaseOption(dbo);

    m->setBaseColorFactor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    m->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
    m->setMetallicFactor(1.00f);
    m->setRoughnessFactor(0.108f);
    m->setAoStrength(0.15f);
    // m->setTextureAttribute(osgEarth::StandardPBRMaterial::NormalMap, "metal/normal.png");
    m->setTextureAttribute(osgEarth::StandardPBRMaterial::BaseColorMap, "metal/albedo.png");
    m->setReceiveEnvLight(true);

    m->addTextureAttribute("metalMap", "metal/metal.png", "OE_ENABLE_Metal_MAP");
    m->addTextureAttribute("roughnessMap", "metal/rough.png", "OE_ENABLE_Roughness_MAP");
    m->setMaterialFile("materials/metalroughness.glsl");
    result.push_back(m);

    osg::ref_ptr<osgEarth::ExtensionedMaterial> gray = new osgEarth::ExtensionedMaterial();
    gray->setName("dalishi");
    gray->setDataBaseOption(dbo);

    gray->setBaseColorFactor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    gray->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
    gray->setMetallicFactor(0.5f);
    gray->setRoughnessFactor(0.176f);
    gray->setAoStrength(0.1f);
    gray->setTextureAttribute(osgEarth::StandardPBRMaterial::NormalMap, "gray/normal.png");
   // gray->setTextureAttribute(osgEarth::StandardPBRMaterial::OcclusionMap, "gray/ao.png");
    gray->setTextureAttribute(osgEarth::StandardPBRMaterial::BaseColorMap, "gray/albedo.png");
    gray->setReceiveEnvLight(true);
    
    gray->addTextureAttribute("metalMap", "gray/metallic.png", "OE_ENABLE_Metal_MAP");
    gray->addTextureAttribute("roughnessMap", "gray/roughness.png", "OE_ENABLE_Roughness_MAP");
    gray->setMaterialFile("materials/metalroughness.glsl");
    result.push_back(gray);

    osg::ref_ptr<osgEarth::ExtensionedMaterial> rock = new osgEarth::ExtensionedMaterial();
    rock->setName("rock");
    rock->setDataBaseOption(dbo);
    
    rock->setBaseColorFactor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    rock->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
    rock->setMetallicFactor(0.5f);
    rock->setRoughnessFactor(0.668f);
    rock->setTextureAttribute(osgEarth::StandardPBRMaterial::NormalMap, "rock/normal.png");
    //rock->setTextureAttribute(osgEarth::StandardPBRMaterial::OcclusionMap, "rock/ao.png");
    rock->setTextureAttribute(osgEarth::StandardPBRMaterial::BaseColorMap, "rock/albedo.png");
    rock->setReceiveEnvLight(true);
    
    rock->addTextureAttribute("metalMap", "rock/ao.png", "OE_ENABLE_Metal_MAP");
    rock->addTextureAttribute("roughnessMap", "rock/roughness.png", "OE_ENABLE_Roughness_MAP");
    rock->setMaterialFile("materials/metalroughness.glsl");
    result.push_back(rock);

    return result;
   
}

osg::ref_ptr<osg::Group> createMaterialSpheres()
{
    osg::ref_ptr<osg::Group> gp = new osg::Group();
   
  
   /* std::vector<osg::ref_ptr<StandardPBRMaterial>> materials = std::move(createNoTexMaterials());
    int row, col;
    row = col = 9;

    for (size_t i = 0; i < row; i++)
    {
        for (size_t j = 0; j < col; j++)
        {
            osg::ref_ptr<osg::Geode> geode = new osg::Geode();
            osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3f(0.0f, 0.0f, 0.0f), 2.0f));
            geode->addDrawable(sd);
            sd->setDrawCallback(new DrawCallback());
            geode->getOrCreateStateSet()->setAttributeAndModes(materials[i * col + j], osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            geode->getOrCreateStateSet()->setName("110");
            osg::ref_ptr<osg::MatrixTransform> matrixT = new osg::MatrixTransform();


            matrixT->setMatrix(osg::Matrix::translate((double)i * 5.0, (double)j * 5.0, 0.0));
            matrixT->addChild(geode);
            matrixT->setName(materials[i * col + j]->getName());

            PBRMaterialCallback().operator()(materials[i*col + j], 0L);
            auto* pbr = new PbrLightEffect();
            pbr->attach(geode->getOrCreateStateSet());
            gp->addChild(matrixT);
        }
       
    }*/
    std::vector<osg::ref_ptr<ExtensionedMaterial>> materials = std::move(createMaterials());
    for (size_t i = 0; i < materials.size(); i++)
    {
        std::cout << " gsfdg "<<i  << std::endl;
        osg::ref_ptr<osg::Geode> geode = new osg::Geode();
        osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3f(0.0f, 0.0f, 0.0f), 2.0f));
        geode->addDrawable(sd);
        geode->getOrCreateStateSet()->setAttributeAndModes(materials[i], osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        osg::ref_ptr<osg::MatrixTransform> matrixT = new osg::MatrixTransform();


        matrixT->setMatrix(osg::Matrix::translate((double)i * 5.0, 0.0, 0.0));
        matrixT->addChild(geode);
        matrixT->setName(materials[i]->getName());

        ExtensionedMaterialCallback().operator()(materials[i], 0L);
        auto* pbr = new PbrLightEffect();
        pbr->attach(geode->getOrCreateStateSet());
        gp->addChild(matrixT);
        

    }
    return gp;
    
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




    osgDB::Registry::instance()->getObjectWrapperManager()->findWrapper("osg::Image");

    auto group = new osg::Group();

    EnvLightEffect::instance()->setEnable(true);

    GLTFReaderV2 reader;
    //Sponza BoomBox
    
    auto gltfModel = reader.read("C:\\Users\\10677\\source\\repos\\OE_PBR\\OE_PBR\\Asset\\BoomBox\\BoomBox.gltf", false, new osgDB::Options("..//..//OE_PBR//Asset//BoomBox"));
    auto node = gltfModel.getNode();
    auto* phong = new PbrLightEffect();
    phong->attach(gltfModel.getNode()->getOrCreateStateSet());
    auto* vp = osgEarth::VirtualProgram::get(gltfModel.getNode()->getOrCreateStateSet());
    vp->setShaderLogging(true);


    auto materialSpheres = createMaterialSpheres();

    group->addChild(createSkyBox());
    group->addChild(materialSpheres);

    //std::vector<std::string> input = { "px.png","nx.png","ny.png","py.png", "pz.png","nz.png" };
    //std::string input = "pisaDiffuseHDR.dds";
    //auto output = "diffuse.dds";
    //auto path = "C:\\Users\\10677\\source\\repos\\OE_PBR\\OE_PBR\\Asset\\IBL\\pisaHDR";
    //osg::ref_ptr<  QuadTextureTransitor > trans = new  QuadTextureTransitor(group, &viewer);
    //trans->translate(path, input, output, 256, 128, 1);
     
    
   


    osg::Light* lightState = new osg::Light;
   auto light = CreateLight(lightState);

   auto materialPanel = new TestGUI();
   auto findCallback = [materialPanel](osg::MatrixTransform* node)
   {
       node->getName();
     //  materialPanel->setNode(node);
       return true;
   };


    auto func = [&](osg::MatrixTransform* node, osg::NodeVisitor* nv)
    {
        auto matrix = node->getMatrix();

        node->setMatrix(matrix * osg::Matrix(osg::Quat(0.01, osg::Vec3(0.0, 0.0, 1.0))));
    };

   // gltfModel.getNode()->addUpdateCallback(new CB<osg::MatrixTransform>(func));
    
   group->addChild(light);


    viewer.setReleaseContextAtEndOfFrameHint(false);

    //viewer.addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));
    viewer.addEventHandler(new RayPicker(&viewer,findCallback));

    // Call this to enable ImGui rendering.
    // If you use the MapNodeHelper, call this first.
    viewer.setRealizeOperation(new GUI::ApplicationGUI::RealizeOperation);

    GUI::ApplicationGUI* gui = new GUI::ApplicationGUI(true);

   
    gui->add("Demo", materialPanel);
    gui->add("Demo2", new LightGUI(lightState));
    gui->add("Demo3", new IndirectLightGUI());
    

    viewer.setSceneData(group);

    viewer.setCameraManipulator(new osgGA::TrackballManipulator);
    viewer.setUpViewInWindow(100, 100, 800, 600);
    viewer.realize();

    viewer.getEventHandlers().push_front(gui);
    Metrics::setEnabled(true);
    Metrics::setGPUProfilingEnabled(true);
    return Metrics::run(viewer);
  /*  viewer.run();*/
    /*return 0;*/
}


