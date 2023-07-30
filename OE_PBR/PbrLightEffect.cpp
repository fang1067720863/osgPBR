
//author:1067720863@qq.com
//create date:2023/04
//decription: PBR Material

#include "PbrLightEffect.h"

#include <osgEarth/PhongLightingEffect>
#include <osgEarth/Registry>
#include <osgEarth/Capabilities>
#include <osgEarth/Shaders>
#include <osgEarth/Lighting>
#include <osgEarth/PhongLightingEffect>
#include <osgEarth/Math>
#include <osgEarth/FileUtils>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgEarth/Registry>
#include <osgEarth/ShaderFactory>
#include<osg/TexEnv>
#include<osg/Texture2DArray>
using namespace osgEarth;
using namespace osgEarth::Util;



class ExtensionedMaterial;


osgEarth::Util::PbrLightEffect::PbrLightEffect()
{
    init();
}

osgEarth::Util::PbrLightEffect::PbrLightEffect(osg::StateSet* stateset)
{
    init();
    attach(stateset);
}


void osgEarth::Util::PbrLightEffect::attach(osg::StateSet* stateset)
{

    if (stateset && _supported)
    {
        _statesets.push_back(stateset);

        // set light
        stateset->setDefine(OE_LIGHTING_DEFINE, "1", osg::StateAttribute::ON);
        stateset->setDefine("OE_NUM_LIGHTS", "1");
        stateset->setDefine("OE_USE_PBR", "1");
        stateset->setDefine("cascade", "1");


        ShaderPackage shaders;
        VirtualProgram* pbrVP = VirtualProgram::getOrCreate(stateset);
        

        osg::ref_ptr<osgDB::Options> dbo = new osgDB::Options();
        
        auto shaderPath = "..//..//OE_PBR//Shader";
        auto shaderPath2 = "..//OE_PBR//Shader";

        if (osgDB::fileExists(osgEarth::getAbsolutePath(shaderPath)))
        {
            dbo->setDatabasePath(osgEarth::getAbsolutePath(shaderPath));
        }
        else {
            dbo->setDatabasePath(osgEarth::getAbsolutePath(shaderPath2));
        }



     
        auto material = stateset->getAttribute(osg::StateAttribute::MATERIAL);

        std::string materialSnippet = "";
        std::string material_defines(""), material_body, material_uniforms("");

        URIContext context(dbo.get());
        URI standard("materials/standard.glsl", context);
        std::string basic = osgDB::findDataFile(standard.full(), dbo);
        material_body = URI(basic, context).getString(dbo);
       
        if (osgEarth::ExtensionedMaterial* mat = dynamic_cast<osgEarth::ExtensionedMaterial*>(material))
        {
            const std::string fileName = mat->materialFile();
            if (!fileName.empty())
            {
                URI uri(fileName, context);
                std::string path = osgDB::findDataFile(uri.full(), dbo);
                if (!path.empty())
                {
                    materialSnippet = URI(path, context).getString(dbo);
                    if (!materialSnippet.empty())
                    {
                        OE_DEBUG << "Loaded materail shader " << fileName << " from " << path << "\n";
                    }
                }
            }
        }
        if (!materialSnippet.empty())
        {
            // parse custom material
            std::string::size_type pragmaPos = 0;
            while (pragmaPos != std::string::npos)
            {
                const std::string token("#pragma import_defines");
                std::string::size_type statementPos = materialSnippet.find(token, pragmaPos);
                if (statementPos == std::string::npos)
                {
                    break;
                }
                std::string::size_type bracketLeft = materialSnippet.find_first_not_of("(", statementPos + token.length());
                std::string::size_type bracketRight = materialSnippet.find_first_of(")", bracketLeft);
                if (!material_defines.empty())
                {
                    material_defines.push_back(',');
                }
                material_defines.append(materialSnippet.substr(bracketLeft, bracketRight - bracketLeft));
                pragmaPos = bracketRight + 1;
            }
            pragmaPos = 0;
            while (pragmaPos != std::string::npos)
            {
                const std::string token("uniform");
                std::string::size_type statementPos = materialSnippet.find(token, pragmaPos);
                if (statementPos == std::string::npos)
                {
                    break;
                }
                std::string::size_type uniformEnd = materialSnippet.find_first_of("\n", statementPos);
                material_uniforms.append(materialSnippet.substr(statementPos, uniformEnd - statementPos));
                pragmaPos = uniformEnd;
            }
            material_body.append(materialSnippet.substr(pragmaPos));

            std::cout << " material_defines " << material_defines << std::endl << "material_define_end" << std::endl;
            std::cout << " material_uniform" << material_uniforms << std::endl << "material_uniform_end" << std::endl;
            std::cout << " material_body" << material_body << std::endl << "material_body_end" << std::endl;
        }
       

        // insert into basic shader
        osgEarth::Registry::instance()->getShaderFactory()->addPreProcessorCallback(
            "MaterialReplace",
            [this, material_body, material_defines, material_uniforms](std::string& output)
            {
                auto replaceFunc = [](const std::string& token, const std::string& source, std::string& output)
                {
                    std::string::size_type statementPos = output.find(token);
                    if (statementPos == std::string::npos)
                        return;

                    std::string::size_type startPos = output.find_first_not_of(" \t", statementPos + token.length());
                    if (startPos == std::string::npos)
                        return;

                    std::string::size_type endPos = output.find('\n', startPos);
                    if (endPos == std::string::npos)
                        return;

                    std::string statement(output.substr(statementPos, endPos - statementPos));

                    Strings::replaceIn(output, statement, source);
                };
                
                replaceFunc("MATERIAL_DEFINES", material_defines, output);
                replaceFunc("# MATERIAL_UNIFORMS", material_uniforms, output);
                replaceFunc("# MATERIAL_BODY", material_body, output);

            }
        );
  
        shaders.load(pbrVP, "pbr.glsl", dbo.get());

    }
}


void osgEarth::Util::PbrLightEffect::detach()
{
    if (_supported)
    {
        for (StateSetList::iterator it = _statesets.begin(); it != _statesets.end(); ++it)
        {
            osg::ref_ptr<osg::StateSet> stateset;
            if ((*it).lock(stateset))
            {
                detach(stateset.get());
                (*it) = 0L;
            }
        }

        _statesets.clear();
    }
}

void osgEarth::Util::PbrLightEffect::detach(osg::StateSet* stateset)
{
    if (stateset && _supported)
    {
        stateset->removeDefine(OE_LIGHTING_DEFINE);

        VirtualProgram* vp = VirtualProgram::get(stateset);
        if (vp)
        {
            Shaders shaders;
            shaders.unload(vp, shaders.PhongLighting);
        }
    }
}

osgEarth::Util::PbrLightEffect::~PbrLightEffect()
{
    detach();
}

void osgEarth::Util::PbrLightEffect::init()
{
    _supported = Registry::capabilities().supportsGLSL();
    _supported = true;
}
