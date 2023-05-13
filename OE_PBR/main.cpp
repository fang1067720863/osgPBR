
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
    osg::Node* _node;
public:
    TestGUI(osg::Node* node) :GUI::BaseGUI("PBR")
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
            
        }
        ImGui::End();
    }
};

osg::ref_ptr<osg::Node> createHDRBox()
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Box(osg::Vec3(0.0, 0.0, 0.0), 16.0));

    geode->addDrawable(sd);
    osg::ref_ptr<osg::MatrixTransform> trans = new MatrixTransform();
    trans->setMatrix(osg::Matrix::scale(16, 16, 16));
    trans->addChild(geode);


    sd->setColor(osg::Vec4(1.0, 0.0, 0.0, 1.0));

    osg::ref_ptr <osg::Image> image = osgDB::readRefImageFile("C:\\Users\\10677\\source\\repos\\OE_PBR\\OE_PBR\\Asset\\abandoned_bakery_4k.hdr");

    osg::ref_ptr<osg::Texture2D> hdr = new osg::Texture2D(image.get());
    geode->getOrCreateStateSet()->setTextureAttributeAndModes(0, hdr.get(), osg::StateAttribute::ON);

    VirtualProgram* vp = VirtualProgram::getOrCreate(geode->getOrCreateStateSet());

    const char* pick_preview = R"(


        #pragma vp_function SampleSphericalMap, vertex_model
        const vec2 invAtan = vec2(0.1591, 0.3183);
        out vec2 spherical_uv;
        void SampleSphericalMap(inout vec4 vertex) {
            vec3 tmp = normalize(vertex.xyz);

            vec2 uv = vec2(atan(tmp.z, tmp.x), asin(tmp.y));
            uv *= invAtan;
            uv += 0.5;
            spherical_uv = uv;
        }

        [break]

         #pragma vp_function fs, fragment_output
        in vec2 spherical_uv;
        out vec4 frag;
        uniform sampler2D tex;
        void fs(inout vec4 c) {
            c = texture(tex, spherical_uv);
            frag = c;
        }
    )";

    ShaderLoader::load(vp, pick_preview);


    return trans;
}
std::vector<osg::ref_ptr<osg::Camera>> setupPreviewCamera(osg::Group* gp)
{

    std::vector<osg::ref_ptr<osg::Camera>> cameras;
    osg::ref_ptr<osg::Node> trans = createHDRBox();

    osg::ref_ptr<osg::Texture2D>previewTexture = new osg::Texture2D();
    double w, h;
    w = h = 256.0;
    previewTexture->setTextureSize(w,h);
    previewTexture->setSourceFormat(GL_RGBA);
    previewTexture->setSourceType(GL_UNSIGNED_BYTE);
    previewTexture->setInternalFormat(GL_RGBA8);

    osg::ref_ptr<osg::TextureCubeMap> cubeMap = new osg::TextureCubeMap();
    cubeMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
    cubeMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);
    cubeMap->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP);
    cubeMap->setTextureSize(w,h);
    cubeMap->setInternalFormat(GL_RGB);
    bool noMipMap, hardwareMipmap;
    noMipMap = hardwareMipmap = false;
    if (noMipMap)
    {
        cubeMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
        cubeMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    }
    else {
        cubeMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        cubeMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    }


    if (hardwareMipmap)
    {
        OSG_NOTICE << "tcm->setUseHardwareMipMapGeneration(true)" << std::endl;
        cubeMap->setUseHardwareMipMapGeneration(true);
    }

    //top 
    {
        osg::ref_ptr<osg::Camera> cam = new osg::Camera();
        cam->addChild(trans);
        cam->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        cam->setClearColor(osg::Vec4(0, 0, 1, 1));
        cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cam->setViewport(0, 0, 256, 256);
        cam->setRenderOrder(osg::Camera::PRE_RENDER);
        cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        cam->setImplicitBufferAttachmentMask(0, 0);
        cam->attach(osg::Camera::COLOR_BUFFER, cubeMap, 0, osg::TextureCubeMap::POSITIVE_X);
        cam->setProjectionMatrixAsOrtho2D(-w / 2, (-w / 2) + w, -h / 2, (-h / 2) + h);
        cam->setViewMatrixAsLookAt(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(1.0, 0.0, 0.0), osg::Vec3(0.0, -1.0, 0.0));
        
        
        cameras.push_back(cam);
    }
    

    {
        osg::ref_ptr<osg::Camera> cam = new osg::Camera();
        cam->addChild(trans);
        cam->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        cam->setClearColor(osg::Vec4(0, 0, 1, 1));
        cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cam->setViewport(0, 0, 256, 256);
        cam->setRenderOrder(osg::Camera::PRE_RENDER);
        cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        cam->setImplicitBufferAttachmentMask(0, 0);
        cam->attach(osg::Camera::COLOR_BUFFER, cubeMap, 0, osg::TextureCubeMap::NEGATIVE_X);
        cam->setProjectionMatrixAsOrtho2D(-w / 2, (-w / 2) + w, -h / 2, (-h / 2) + h);
        cam->setViewMatrixAsLookAt(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(-1.0, 0.0, 0.0), osg::Vec3(0.0, -1.0, 0.0));
        cameras.push_back(cam);
    }
    {
        osg::ref_ptr<osg::Camera> cam = new osg::Camera();
        cam->addChild(trans);
        cam->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        cam->setClearColor(osg::Vec4(0, 0, 1, 1));
        cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cam->setViewport(0, 0, 256, 256);
        cam->setRenderOrder(osg::Camera::PRE_RENDER);
        cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        cam->setImplicitBufferAttachmentMask(0, 0);
        cam->attach(osg::Camera::COLOR_BUFFER, cubeMap, 0, osg::TextureCubeMap::POSITIVE_Y);
        cam->setProjectionMatrixAsOrtho2D(-w / 2, (-w / 2) + w, -h / 2, (-h / 2) + h);
        //cam->setViewMatrixAsLookAt(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 1.0, 0.0), osg::Vec3(0.0, 0.0, 1.0));
        cam->setViewMatrix(osg::Matrixd());
        cameras.push_back(cam);
    }
    {
        osg::ref_ptr<osg::Camera> cam = new osg::Camera();
        cam->addChild(trans);
        cam->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        cam->setClearColor(osg::Vec4(0, 0, 1, 1));
        cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cam->setViewport(0, 0, 256, 256);
        cam->setRenderOrder(osg::Camera::PRE_RENDER);
        cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        cam->setImplicitBufferAttachmentMask(0, 0);
        cam->attach(osg::Camera::COLOR_BUFFER, cubeMap, 0, osg::TextureCubeMap::NEGATIVE_Y);
        cam->setProjectionMatrixAsOrtho2D(-w / 2, (-w / 2) + w, -h / 2, (-h / 2) + h);
        cam->setViewMatrixAsLookAt(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, -1.0, 0.0), osg::Vec3(0.0, -1.0, 0.0));
        cam->setViewMatrix(osg::Matrixd::rotate(osg::inDegrees(180.0f), 1.0, 0.0, 0.0));
        cameras.push_back(cam);
    }
    {
        osg::ref_ptr<osg::Camera> cam = new osg::Camera();
        cam->addChild(trans);
        cam->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        cam->setClearColor(osg::Vec4(0, 0, 1, 1));
        cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cam->setViewport(0, 0, 256, 256);
        cam->setRenderOrder(osg::Camera::PRE_RENDER);
        cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        cam->setImplicitBufferAttachmentMask(0, 0);
        cam->attach(osg::Camera::COLOR_BUFFER, cubeMap, 0, osg::TextureCubeMap::POSITIVE_Z);
        cam->setProjectionMatrixAsOrtho2D(-w / 2, (-w / 2) + w, -h / 2, (-h / 2) + h);
        cam->setViewMatrix(osg::Matrixd::rotate(osg::inDegrees(-90.0f), 1.0, 0.0, 0.0));
        //cam->setViewMatrixAsLookAt(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, 1.0), osg::Vec3(0.0, -1.0, 0.0));
        cameras.push_back(cam);
    }
    {
        osg::ref_ptr<osg::Camera> cam = new osg::Camera();
        cam->addChild(trans);
        cam->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        cam->setClearColor(osg::Vec4(0, 0, 1, 1));
        cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cam->setViewport(0, 0, 256, 256);
        cam->setRenderOrder(osg::Camera::PRE_RENDER);
        cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        cam->setImplicitBufferAttachmentMask(0, 0);
        cam->attach(osg::Camera::COLOR_BUFFER, cubeMap, 0, osg::TextureCubeMap::NEGATIVE_Z);
        cam->setProjectionMatrixAsOrtho2D(-w / 2, (-w / 2) + w, -h / 2, (-h / 2) + h);
        //cam->setViewMatrixAsLookAt(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, -1.0), osg::Vec3(0.0, -1.0, 0.0));
        cam->setViewMatrix(osg::Matrixd::rotate(osg::inDegrees(90.0f), 1.0, 0.0, 0.0) * osg::Matrixd::rotate(osg::inDegrees(180.0f), 0.0, 0.0, 1.0));
        cameras.push_back(cam);
    }

    //{
    //    osg::ref_ptr<osg::Geode> geode = new osg::Geode();

    //   
    //    osg::StateSet* stateset = geode->getOrCreateStateSet();
    //    stateset->setTextureAttributeAndModes(0, cubeMap, osg::StateAttribute::ON);
    //    stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
    //    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    //    float characterSize = 20.0f;
    //    osg::Vec3 pos = osg::Vec3(0.0f, 0.0f, 0.0f);
    //    osg::Vec3 width(characterSize, 0.0f, 0.0);
    //    osg::Vec3 height(0.0f, 0.0f, characterSize * (256.0f) / (256.0f));

    //    osg::Geometry* geometry = osg::createTexturedQuadGeometry(pos, width, height);

    //    osg::Geode* geode2 = new osg::Geode;
    //    geode2->addDrawable(geometry);

    //    gp->addChild(geode2);

    //   
   // }

    return cameras;
}



osg::ref_ptr<osg::Node> CreatePbrSphere()
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();

    osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3f(0.0f, 0.0f, 0.0f), 2.0f));

    geode->addDrawable(sd);
   
    bool usePhong = true;
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

    }
   
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
    myLight1->setSpotExponent(50.0f);
    myLight1->setDirection(osg::Vec3(0.382353f, 0.254902f, 0.382353f));

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

    //auto node = CreatePbrSphere();

    GLTFReaderV2 reader;
    //Sponza BoomBox
    
    auto gltfModel = reader.read("C:\\Users\\10677\\source\\repos\\OE_PBR\\OE_PBR\\Asset\\BoomBox\\BoomBox.gltf", false, new osgDB::Options("..//..//OE_PBR//Asset//BoomBox"));
    auto node = gltfModel.getNode();
    auto* phong = new PbrLightEffect();
    phong->attach(gltfModel.getNode()->getOrCreateStateSet());
    auto* vp = osgEarth::VirtualProgram::get(gltfModel.getNode()->getOrCreateStateSet());
    vp->setShaderLogging(true);

   // group->addChild(gltfModel.getNode());
    osg::Light* lightState = new osg::Light;
    auto light = CreateLight(gltfModel.getNode()->getOrCreateStateSet(), lightState);

  /*  auto cameras = std::move(setupPreviewCamera(group));
    for (auto cam : cameras)
    {
        group->addChild(cam);
    }*/
    osg::ref_ptr<IBLTechnique> ibl = new IBLTechnique(group,&viewer);
    ibl->startUp();
    group->addChild(createHDRBox());


    auto func = [&](osg::MatrixTransform* node, osg::NodeVisitor* nv)
    {
        auto matrix = node->getMatrix();

        node->setMatrix(matrix * osg::Matrix(osg::Quat(0.01, osg::Vec3(0.0, 0.0, 1.0))));
    };

   // gltfModel.getNode()->addUpdateCallback(new CB<osg::MatrixTransform>(func));
    
    group->addChild(light);

    //group->addChild(node);
    viewer.setReleaseContextAtEndOfFrameHint(false);

    viewer.addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));

    // Call this to enable ImGui rendering.
    // If you use the MapNodeHelper, call this first.
    viewer.setRealizeOperation(new GUI::ApplicationGUI::RealizeOperation);

    GUI::ApplicationGUI* gui = new GUI::ApplicationGUI(true);
    gui->add("Demo", new TestGUI(gltfModel.getNode()));
    gui->add("Demo2", new LightGUI(lightState));

    viewer.setSceneData(group);

    viewer.setCameraManipulator(new osgGA::TrackballManipulator);
    viewer.setUpViewInWindow(100, 100, 800, 600);
    viewer.realize();

    viewer.getEventHandlers().push_front(gui);
    viewer.run();
    return 0;
}


