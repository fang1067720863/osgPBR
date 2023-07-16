
//#define NOMINMAX
#include<Windows.h>

#include <osgEarth/ImGui/ImGuiApp>

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
    char material[256] = "Original text";
    osg::Node* _node;
public:
    TestGUI(osg::Node* node) :GUI::BaseGUI("PBR Material")
    {
        _node = node;
        UniformSpec metallic{ "oe_pbr.metallicFactor" ,0.0f,1.0f,0.5f };
        UniformSpec roughness{ "oe_pbr.roughnessFactor" ,0.0f,1.0f,0.5f };
        UniformSpec aoStrength{ "oe_pbr.aoStrength" ,0.0f,1.0f,0.5f };
        DefineSpec normal{ "OE_ENABLE_NORMAL_MAP" , "3",true};
        DefineSpec mr{ "OE_ENABLE_MR_MAP" ,"4", true};
        DefineSpec ao{ "OE_ENABLE_AO_MAP" ,"2", true};
        DefineSpec emssive{ "OE_ENABLE_EMISSIVE_MAP" ,"1", true};
        DefineSpec baseColor{ "OE_ENABLE_BASECOLOR_MAP" ,"0", true};

        _uniforms.emplace_back(metallic);
        _uniforms.emplace_back(roughness);
        _uniforms.emplace_back(aoStrength);

        _defines.emplace_back(normal);
        _defines.emplace_back(mr);
        _defines.emplace_back(ao);
        _defines.emplace_back(emssive);
        _defines.emplace_back(baseColor);

    }
    bool setNode(osg::MatrixTransform* node)
    {
        osg::Node* child = node->getChild(0);
        strcpy(material, node->getName().c_str());
        _node = child;
        for (auto& def : _uniforms)
        {
            if (def._name == "oe_pbr.metallicFactor")
            {
                float metallicFactor;
                child->getOrCreateStateSet()->getUniform("oe_pbr.metallicFactor")->get(metallicFactor);
                def._value = metallicFactor;
            }
            if (def._name == "oe_pbr.roughnessFactor")
            {
                float roughnessFactor;
                child->getOrCreateStateSet()->getUniform("oe_pbr.roughnessFactor")->get(roughnessFactor);
                def._value = roughnessFactor;
            }
            if (def._name == "oe_pbr.aoStrength")
            {
                float aoStrength;
                child->getOrCreateStateSet()->getUniform("oe_pbr.aoStrength")->get(aoStrength);
                def._value = aoStrength;
            }
        }

        return true;
    }

    void draw(osg::RenderInfo& ri)override {
        if (!isVisible()) {
            return;
        }
        ImGui::Begin(name(), visible());
        {
            if (!_uniforms.empty())
            {
                ImGui::Text(material);
            }
        }
        for (auto& def : _uniforms)
        {
            if (ImGui::SliderFloat(def._name.c_str(), &def._value, def._minval, def._maxval))
            {
                _node->getOrCreateStateSet()->getOrCreateUniform(def._name, osg::Uniform::FLOAT)->set(def._value);
                _node->getOrCreateStateSet()->addUniform(new osg::Uniform(def._name.c_str(), def._value), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
               
            }
        }
        for (auto& def : _defines)
        {
            if (ImGui::Checkbox(def._name.c_str(), &def._checked))
            {

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


class LightGUI :public osgEarth::GUI::BaseGUI
{
public:

    osg::Light* _light;
public:
    LightGUI(osg::Light* light) :GUI::BaseGUI("Light"), _light(light)
    {
       

    }

    void draw(osg::RenderInfo& ri)override {
        if (!isVisible()) {
            return;
        }
        ImGui::Begin(name(), visible());


        ImGui::Separator();
        {
            //IMGUI_DEMO_MARKER("Widgets/Basic/ColorEdit3, ColorEdit4");
            auto ambient = _light->getAmbient();
            auto diffuse = _light->getDiffuse();
            auto direction = _light->getDirection();
            static float col1[4] = { ambient[0],ambient[1], ambient[2], ambient[3] };
            static float col2[4] = { diffuse[0],diffuse[1], diffuse[2], diffuse[3] };
            static float col3[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
            static float col4[3] = { direction[0],direction[1], direction[2] };
            static float itensity = 0.5f;
            static bool lightenable = true;
            if (ImGui::Checkbox("on/off", &lightenable))
            {
                if (lightenable)
                {
                    _light->setAmbient(osg::Vec4(col1[0], col1[1], col1[2], col1[3]));
                    _light->setDiffuse(osg::Vec4(col2[0], col2[1], col2[2], col2[3]));
                    _light->setDirection(osg::Vec3(col4[0], col4[1], col4[2]));
                }
                else {
                    _light->setAmbient(osg::Vec4(0.0, 0.0, 0.0, 0.0));
                    _light->setDiffuse(osg::Vec4(0.0, 0.0, 0.0, 0.0));
                    _light->setDirection(osg::Vec3(0.0, 0.0, 0.0));
                }
            }
            if (ImGui::ColorEdit4("ambient", col1))
            {
                _light->setAmbient(osg::Vec4(col1[0], col1[1], col1[2], col1[3]));
            }
            if (ImGui::ColorEdit4("diffuse", col2))
            {
                _light->setDiffuse(osg::Vec4(col2[0], col2[1], col2[2], col2[3]));
            }
            if (ImGui::ColorEdit4("direction", col4))
            {
                _light->setDirection(osg::Vec3(col4[0], col4[1], col4[2]));
                std::cout << "setDirection" << col4[0] << " " << col4[1] << " " << col4[2] << std::endl;
            }
            if (ImGui::SliderFloat("intensity", &itensity, 0.0f, 1.0f))
            {
                _light->setSpotExponent(itensity);
              
            }

            
        }
        ImGui::End();
    }
};


class IndirectLightGUI :public osgEarth::GUI::BaseGUI
{
public:

    osg::Light* _light;
public:
    IndirectLightGUI(osg::Light* light) :GUI::BaseGUI("IndirectLightGUI"), _light(light)
    {


    }

    void draw(osg::RenderInfo& ri)override {
        if (!isVisible()) {
            return;
        }
        ImGui::Begin(name(), visible());


        ImGui::Separator();
        {
            //IMGUI_DEMO_MARKER("Widgets/Basic/ColorEdit3, ColorEdit4");
            auto ambient = _light->getAmbient();
            auto diffuse = _light->getDiffuse();
            auto direction = _light->getDirection();
            static float col1[4] = { ambient[0],ambient[1], ambient[2], ambient[3] };
            static float col2[4] = { diffuse[0],diffuse[1], diffuse[2], diffuse[3] };
            static float col3[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
            static float col4[3] = { direction[0],direction[1], direction[2] };
            static float itensity = 0.5f;
            static bool lightenable = true;
            if (ImGui::Checkbox("on/off", &lightenable))
            {
                if (lightenable)
                {
                    _light->setAmbient(osg::Vec4(col1[0], col1[1], col1[2], col1[3]));
                    _light->setDiffuse(osg::Vec4(col2[0], col2[1], col2[2], col2[3]));
                    _light->setDirection(osg::Vec3(col4[0], col4[1], col4[2]));
                }
                else {
                    _light->setAmbient(osg::Vec4(0.0, 0.0, 0.0, 0.0));
                    _light->setDiffuse(osg::Vec4(0.0, 0.0, 0.0, 0.0));
                    _light->setDirection(osg::Vec3(0.0, 0.0, 0.0));
                }
            }
            if (ImGui::SliderFloat("intensity", &itensity, 0.0f, 1.0f))
            {
                _light->setSpotExponent(itensity);

            }


        }
        ImGui::End();
    }
};

float _uroughnessToMip( float _uroughness) {
    float _umip = 0.0;
    if ((_uroughness >= 0.80000001))
    {
        (_umip = ((((1.0 - _uroughness) * 1.0) / 0.19999999) + -2.0));
    }
    else
    {
        if ((_uroughness >= 0.40000001))
        {
            (_umip = ((((0.80000001 - _uroughness) * 3.0) / 0.40000001) + -1.0));
        }
        else
        {
            if ((_uroughness >= 0.30500001))
            {
                (_umip = ((((0.40000001 - _uroughness) * 1.0) / 0.094999999) + 2.0));
            }
            else
            {
                if ((_uroughness >= 0.20999999))
                {
                    (_umip = ((((0.30500001 - _uroughness) * 1.0) / 0.095000014) + 3.0));
                }
                else
                {
                    (_umip = (-2.0 * log2((1.16 * _uroughness))));
                }
            }
        }
    }
    return _umip;
}

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
osg::Node* CreateLight(osg::StateSet* rootStateSet, osg::Light * myLight1)
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

    lightS1->setStateSetModes(*rootStateSet, osg::StateAttribute::ON);
    lightGroup->addChild(lightS1);
#ifdef NDEBUG
    lightS1->addCullCallback(new LightSourceGL3UniformGenerator());
#endif 
    return lightGroup;
}

std::vector<osg::ref_ptr<StandardPBRMaterial>> createNoTexMaterials()
{
    std::vector<osg::ref_ptr<StandardPBRMaterial>> result;

    for (int i = 1; i <= 8; i++)
    {
        for (int j = 1; j <= 8; j++)
        {
            osg::ref_ptr<osgEarth::StandardPBRMaterial> m = new osgEarth::StandardPBRMaterial();

            m->setBaseColorFactor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
            m->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
            m->setMetallicFactor(0.11f * i);
            m->setRoughnessFactor(0.11f * j);
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


    osg::ref_ptr<osgEarth::ExtensionedMaterial> m = new osgEarth::ExtensionedMaterial();
    m->setName("metal");
    m->setDataBaseOption(dbo);

    m->setBaseColorFactor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    m->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
    m->setMetallicFactor(1.00f);
    m->setRoughnessFactor(0.108f);
    m->setAoStrength(0.15f);
    m->setTextureAttribute(osgEarth::StandardPBRMaterial::NormalMap, "metal/normal.png");
    m->setReceiveEnvLight(true);

    m->addTextureAttribute("metalMap",  "metal/metal.png", "OE_ENABLE_Metal_MAP");
    m->addTextureAttribute("roughnessMap",  "metal/rough.png", "OE_ENABLE_Roughness_MAP");
    m->setMaterialFile("materials/metalroughness.glsl");
    result.push_back(m);
    

    osg::ref_ptr<osgEarth::ExtensionedMaterial> grass = new osgEarth::ExtensionedMaterial();
    grass->setName("grass");
    grass->setDataBaseOption(dbo);
   
    grass->setBaseColorFactor(osg::Vec4f(0.5f, 0.5f, 0.5f, 0.5f));
    grass->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
    grass->setMetallicFactor(0.29f);
    grass->setRoughnessFactor(0.81f);
    grass->setAoStrength(0.15f);
    grass->setTextureAttribute(osgEarth::StandardPBRMaterial::NormalMap, "grass/normal.png");
    grass->setTextureAttribute(osgEarth::StandardPBRMaterial::OcclusionMap, "grass/ao.png");
    grass->setTextureAttribute(osgEarth::StandardPBRMaterial::BaseColorMap, "grass/albedo.png");
    grass->setReceiveEnvLight(true);
    
    grass->addTextureAttribute("metalMap", "grass/metallic.png", "OE_ENABLE_Metal_MAP");
    grass->addTextureAttribute("roughnessMap", "grass/roughness.png", "OE_ENABLE_Roughness_MAP");
    grass->setMaterialFile("materials/metalroughness.glsl");
    result.push_back(grass);


    osg::ref_ptr<osgEarth::ExtensionedMaterial> gray = new osgEarth::ExtensionedMaterial();
    gray->setName("dalishi");
    gray->setDataBaseOption(dbo);

    gray->setBaseColorFactor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    gray->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
    gray->setMetallicFactor(0.5f);
    gray->setRoughnessFactor(0.668f);
    //gray->setAoStrength(0.1f);
    gray->setTextureAttribute(osgEarth::StandardPBRMaterial::NormalMap, "gray/normal.png");
    //gray->setTextureAttribute(osgEarth::StandardPBRMaterial::OcclusionMap, "gray/ao.png");
    gray->setTextureAttribute(osgEarth::StandardPBRMaterial::BaseColorMap, "gray/albedo.png");
    gray->setReceiveEnvLight(true);
    
    gray->addTextureAttribute("metalMap", "grass/metallic.png", "OE_ENABLE_Metal_MAP");
    gray->addTextureAttribute("roughnessMap", "grass/roughness.png", "OE_ENABLE_Roughness_MAP");
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
    rock->setReceiveEnvLight(false);
    
    rock->addTextureAttribute("metalMap", "rock/metallic.png", "OE_ENABLE_Metal_MAP");
    rock->addTextureAttribute("roughnessMap", "rock/roughness.png", "OE_ENABLE_Roughness_MAP");
    rock->setMaterialFile("materials/metalroughness.glsl");
    result.push_back(rock);

    return result;
   
}

osg::ref_ptr<osg::Group> createMaterialSpheres()
{
    osg::ref_ptr<osg::Group> gp = new osg::Group();
   
  

    //size_t cnt = materials.size();
  /*  std::vector<osg::ref_ptr<StandardPBRMaterial>> materials = std::move(createNoTexMaterials());
    int row, col;
    row = col = 8;*/
 /*   for (size_t i = 0; i < 8; i++)
    {
        for (size_t j = 0; j < 8; j++)
        {
            osg::ref_ptr<osg::Geode> geode = new osg::Geode();
            osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3f(0.0f, 0.0f, 0.0f), 2.0f));
            geode->addDrawable(sd);
            geode->getOrCreateStateSet()->setAttributeAndModes(materials[i * col + j], osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

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

 //   group->addChild(gltfModel.getNode());
    auto sphere = CreatePbrSphere();

    auto materialSpheres = createMaterialSpheres();


    group->addChild(materialSpheres.get());


    osg::Light* lightState = new osg::Light;
   auto light = CreateLight(sphere->getOrCreateStateSet(), lightState);

   auto materialPanel = new TestGUI(materialSpheres.get());
   auto findCallback = [materialPanel](osg::MatrixTransform* node)
   {
    /*   auto matrix = node->getMatrix();

       node->setMatrix(matrix * osg::Matrix(osg::Quat(0.01, osg::Vec3(0.0, 0.0, 1.0))));*/

       node->getName();
       materialPanel->setNode(node);
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

    viewer.setSceneData(group);

    viewer.setCameraManipulator(new osgGA::TrackballManipulator);
    viewer.setUpViewInWindow(100, 100, 800, 600);
    viewer.realize();

    viewer.getEventHandlers().push_front(gui);
    viewer.run();
    return 0;
}


