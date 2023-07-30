#pragma once
#include <osgEarth/ImGui/ImGuiApp>
#include"EnvLight.h"



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
    TestGUI() :GUI::BaseGUI("PBR Material")
    {
        _node = nullptr;
        UniformSpec metallic{ "oe_pbr.metallicFactor" ,0.0f,1.0f,0.5f };
        UniformSpec roughness{ "oe_pbr.roughnessFactor" ,0.0f,1.0f,0.5f };
        UniformSpec aoStrength{ "oe_pbr.aoStrength" ,0.0f,1.0f,0.5f };
        DefineSpec normal{ "OE_ENABLE_NORMAL_MAP" , "3",true };
        DefineSpec mr{ "OE_ENABLE_MR_MAP" ,"4", true };
        DefineSpec ao{ "OE_ENABLE_AO_MAP" ,"2", true };
        DefineSpec emssive{ "OE_ENABLE_EMISSIVE_MAP" ,"1", true };
        DefineSpec baseColor{ "OE_ENABLE_BASECOLOR_MAP" ,"0", true };

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
                if (_node)
                {
                    _node->getOrCreateStateSet()->getOrCreateUniform(def._name, osg::Uniform::FLOAT)->set(def._value);
                    _node->getOrCreateStateSet()->addUniform(new osg::Uniform(def._name.c_str(), def._value), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                }


            }
        }
        for (auto& def : _defines)
        {
            if (ImGui::Checkbox(def._name.c_str(), &def._checked))
            {
                if (_node)
                {
                    if (def._checked)
                    {
                        _node->getOrCreateStateSet()->setDefine(def._name, def._val, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                    }
                    else {
                        _node->getOrCreateStateSet()->setDefine(def._name, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
                    }
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
    IndirectLightGUI() :GUI::BaseGUI("IndirectLightGUI")
    {


    }

    void draw(osg::RenderInfo& ri)override {
        if (!isVisible()) {
            return;
        }
        ImGui::Begin(name(), visible());


        ImGui::Separator();
        {

            static float itensity = 0.5f;
            static bool lightenable = true;
            if (ImGui::Checkbox("on/off", &lightenable))
            {
                EnvLightEffect::instance()->setEnable(lightenable);
            }
            if (ImGui::SliderFloat("intensity", &itensity, 0.0f, 1.0f))
            {
                std::cout << "itensity" << itensity << std::endl;
                EnvLightEffect::instance()->setLightIntensity(itensity);

            }

            const char* items[] = { "pisaHDR", "abandoned_bakery_4k" };
            static int item_current = 0;
            if (ImGui::Combo("EnvMap", &item_current, items, IM_ARRAYSIZE(items)))
            {
                if (item_current == 0)
                {
                    EnvLightEffect::instance()->setEnvMapAtlas({ "pisaHDR\\diffuse.png", "pisaHDR\\specular.dds", "pisaHDR\\env.dds" });
                }
                else {

                    EnvLightEffect::instance()->setEnvMapAtlas({ "bakery\\diffuse_quad.png", "bakery\\specular_quad.dds", "bakery\\abandoned_bakery_4k.hdr" });
                }
              
            }
           
            


        }
        ImGui::End();
    }
};
