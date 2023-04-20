
//#define NOMINMAX
#include<Windows.h>

#include <osgEarth/ImGui/ImGuiApp>

#include<osg/GLExtensions>
#include <osgViewer/Viewer>
//#include <osgGA/FirstPersonManipulator>
#include<osg/ShapeDrawable>
#include<osgGA/TrackballManipulator>
//#include <osgGA/DriveManipulator>
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

#include"PbrLightEffect.h"
#include"PbrMaterial.h"
#include"GLTFV2Reader.h"
#include"FlyCameraManipulator.h"

//#define LC "[viewer] "

//#ifndef OSG_GL_FIXED_FUNCTION_AVAILABLE
//#define OSG_GL_FIXED_FUNCTION_AVAILABLE
//#endif // !1



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

class TestGUI :public osgEarth::GUI::BaseGUI
{
public:
    float metal = 1.0;
    float roughness = 0.0;
    struct UniformSpec {
        std::string _name;
        float _minval, _maxval, _value;
        //osg::ref_ptr<osg::Uniform> _u;
    };
    

    struct DefineSpec {
        std::string _name;
        std::string _val;
        bool _checked;
    };
    std::vector<DefineSpec> _defines;
    std::vector<UniformSpec> _uniforms;
    osg::Node* _node;
public:
    TestGUI(osg::Node* node) :GUI::BaseGUI("PBR")
    {
        _node = node;
        UniformSpec metallic{ "oe_pbr.metallicFactor" ,0.0f,1.0f,0.5f };
        UniformSpec roughness{ "oe_pbr.roughnessFactor" ,0.0f,1.0f,0.5f };
        DefineSpec normal{ "OE_ENABLE_NORMAL_MAP" , "3",true};
        DefineSpec mr{ "OE_ENABLE_MR_MAP" ,"4", true};
        DefineSpec ao{ "OE_ENABLE_AO_MAP" ,"2", true};
        DefineSpec emssive{ "OE_ENABLE_EMISSIVE_MAP" ,"1", true};
        DefineSpec baseColor{ "OE_ENABLE_BASECOLOR_MAP" ,"0", true};

        _uniforms.emplace_back(metallic);
        _uniforms.emplace_back(roughness);

        _defines.emplace_back(normal);
        _defines.emplace_back(mr);
        _defines.emplace_back(ao);
        _defines.emplace_back(emssive);
        _defines.emplace_back(baseColor);

    }

    void draw(osg::RenderInfo& ri)override {
        if (!isVisible()) {
            return;
        }
        ImGui::Begin(name(), visible());
        {
            if (!_uniforms.empty())
            {
                ImGui::Text("Uniforms:");
            }
        }
        for (auto& def : _uniforms)
        {
            if (ImGui::SliderFloat(def._name.c_str(), &def._value, def._minval, def._maxval))
            {
                //std::cout << "def._value" << def._value << std::endl;
               
                _node->getOrCreateStateSet()->getOrCreateUniform(def._name, osg::Uniform::FLOAT)->set(def._value);
                _node->getOrCreateStateSet()->addUniform(new osg::Uniform(def._name.c_str(), def._value), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
               

               // _node->getOrCreateStateSet()->getAttribute(osgEarth::StandardPBRMaterial::SA_TYPE)
            }
        }
        for (auto& def : _defines)
        {
            if (ImGui::Checkbox(def._name.c_str(), &def._checked))
            {
                //std::cout << "def._value" << def._val << std::endl;

                if (def._checked)
                {
                    _node->getOrCreateStateSet()->setDefine(def._name, def._val, osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);
                }
                else {
                    _node->getOrCreateStateSet()->setDefine(def._name, osg::StateAttribute::OFF|osg::StateAttribute::OVERRIDE);
                }
            }
        }
        ImGui::End();
    }
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
        PBRMaterialCallback().operator()(m, 0L);

        auto* pbr = new PbrLightEffect();
        pbr->attach(geode->getOrCreateStateSet());
        auto* vp = osgEarth::VirtualProgram::get(geode->getOrCreateStateSet());
        vp->setShaderLogging(true);

    }
    

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
    myLight1->setAmbient(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    myLight1->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    myLight1->setSpotCutoff(20.0f);
    myLight1->setSpotExponent(50.0f);
    myLight1->setDirection(osg::Vec3(1.0f, 1.0f, -1.0f));

    osg::LightSource* lightS1 = new osg::LightSource;
    lightS1->setLight(myLight1);
    lightS1->setLocalStateSetModes(osg::StateAttribute::ON);

    lightS1->setStateSetModes(*rootStateSet, osg::StateAttribute::ON);
    lightGroup->addChild(lightS1);


    // create a local light.
    //osg::Light* myLight2 = new osg::Light;
    //myLight2->setLightNum(1);
    //myLight2->setPosition(osg::Vec4(0.0, 0.0, 0.0, 1.0f));
    //myLight2->setAmbient(osg::Vec4(0.0f, 1.0f, 1.0f, 1.0f));
    //myLight2->setDiffuse(osg::Vec4(0.0f, 1.0f, 1.0f, 1.0f));
    //myLight2->setConstantAttenuation(1.0f);
    //myLight2->setLinearAttenuation(2.0f / modelSize);
    //myLight2->setQuadraticAttenuation(2.0f / osg::square(modelSize));

    //osg::LightSource* lightS2 = new osg::LightSource;
    //lightS2->setLight(myLight2);
    //lightS2->setLocalStateSetModes(osg::StateAttribute::ON);

    //lightS2->setStateSetModes(*rootStateSet, osg::StateAttribute::ON);
    //lightGroup->addChild(lightS2);

#ifdef NDEBUG
    GenerateGL3LightingUniforms gen;
    std::cout << "========================================";
    lightGroup->accept(gen);

#endif // DEBUG


 /*   _lightSource->setCullingActive(false);*/
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

    //auto node = CreatePbrSphere();

    GLTFReaderV2 reader;
    //Sponza BoomBox
    auto gltfModel = reader.read("C:\\Users\\10677\\source\\repos\\OE_PBR\\OE_PBR\\Asset\\BoomBox\\BoomBox.gltf", false,new osgDB::Options("..//..//OE_PBR//Asset//BoomBox"));
    auto node = gltfModel.getNode();
    auto* phong = new PbrLightEffect();
    phong->attach(gltfModel.getNode()->getOrCreateStateSet());
    auto* vp = osgEarth::VirtualProgram::get(gltfModel.getNode()->getOrCreateStateSet());
    vp->setShaderLogging(true);

    group->addChild(gltfModel.getNode());
    auto light = CreateLight(gltfModel.getNode()->getOrCreateStateSet());

    
    group->addChild(light);

    //group->addChild(node);
    viewer.setReleaseContextAtEndOfFrameHint(false);

    // Call this to enable ImGui rendering.
    // If you use the MapNodeHelper, call this first.
    viewer.setRealizeOperation(new GUI::ApplicationGUI::RealizeOperation);

    GUI::ApplicationGUI* gui = new GUI::ApplicationGUI(true);
    gui->add("Demo", new TestGUI(gltfModel.getNode()));
   

    viewer.setSceneData(group);

    viewer.setCameraManipulator(new osgGA::TrackballManipulator);
    osgUtil::Optimizer opt;
    //opt.optimize(node, osgUtil::Optimizer::INDEX_MESH);
    //ShaderGenerator gen;
    //node->accept(gen);
    viewer.setUpViewInWindow(100, 100, 800, 600);
    viewer.realize();

    viewer.getEventHandlers().push_front(gui);
    viewer.run();
    return 0;
}


