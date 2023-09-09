#include<Windows.h>
#include"AdvancedMaterial.h"
namespace osgEarth
{
	const std::string AdvancedMaterial::USE_IOR = std::string("USE_IOR");
	const std::string AdvancedMaterial::USE_SHEEN = std::string("USE_SHEEN");
	const std::string AdvancedMaterial::USE_CLEARCOAT = std::string("USE_CLEARCOAT");
	const std::string AdvancedMaterial::USE_IRIDESCENCE = std::string("USE_IRIDESCENCE");
	const std::string AdvancedMaterial::USE_TRANSMISSION = std::string("USE_TRANSMISSION");

	void AdvancedMaterial::initTechniqueCallback()
	{
		auto sheenCB = [](osg::StateAttribute* attr, osg::NodeVisitor* nv)
		{
            osgEarth::AdvancedMaterial* material = static_cast<osgEarth::AdvancedMaterial*>(attr);
			for (unsigned int i = 0; i < attr->getNumParents(); i++)
			{
				osg::StateSet* stateSet = attr->getParent(i);
				auto on = material->getUseSheen() ? osg::StateAttribute::ON : osg::StateAttribute::OFF;
				stateSet->setDefine(USE_SHEEN, "1", on);
				if (!material->getUseSheen())
				{
					return;
				}

				static const std::string SHEENCOLOR = UPREFIX "sheenColor";
				static const std::string SHEENROUGHNESS = UPREFIX "sheenRoughness";
				stateSet->getOrCreateUniform(SHEENCOLOR, osg::Uniform::FLOAT_VEC3)->set(material->getSheenColor());
				stateSet->getOrCreateUniform(SHEENROUGHNESS, osg::Uniform::FLOAT)->set(material->getSheenRoughness());

				auto sheenColorMap = material->getSheenColorMap();
				auto sheenRoughnessMap	= material->getSheenRoughnessMap();
				if (sheenColorMap.valid())
				{
					//auto defineKey = textureDefines[TextureEnum::SHEENCOLORMAP];
					stateSet->setTextureAttributeAndModes(material->texUnitCnt(), material->getSheenColorMap(), osg::StateAttribute::ON);
					stateSet->getOrCreateUniform("SheenColorMap", osg::Uniform::SAMPLER_2D)->set(material->texUnitCnt());
					material->incementTexUnit();

				}
				if (sheenRoughnessMap.valid())
				{
					stateSet->setTextureAttributeAndModes(material->texUnitCnt(), material->getSheenRoughnessMap(), osg::StateAttribute::ON);
					stateSet->getOrCreateUniform("SheenRoughnessMap", osg::Uniform::SAMPLER_2D)->set(material->texUnitCnt());
					material->incementTexUnit();
				}	
			}

		};

		auto clearCoatCB = [](osg::StateAttribute* attr, osg::NodeVisitor* nv)
		{
			osgEarth::AdvancedMaterial* material = static_cast<osgEarth::AdvancedMaterial*>(attr);
			for (unsigned int i = 0; i < attr->getNumParents(); i++)
			{
				osg::StateSet* stateSet = attr->getParent(i);
				auto on = material->getUseClearcoat() ? osg::StateAttribute::ON : osg::StateAttribute::OFF;
				stateSet->setDefine(USE_CLEARCOAT, "1", on);
				if (!material->getUseClearcoat())
				{
					return;
				}
			
				stateSet->getOrCreateUniform(UPREFIX "clearcoat", osg::Uniform::FLOAT)->set(material->getClearcoat());
				stateSet->getOrCreateUniform(UPREFIX "clearcoatRoughness", osg::Uniform::FLOAT)->set(material->getClearcoatRoughness());

			}

		};
		
		_techniqueCallbacks = new SAttrCallback();
		_techniqueCallbacks->addUpdateCallback("pbr", new PBRMaterialCallback());
		_techniqueCallbacks->addUpdateCallback("sheen", sheenCB);
		_techniqueCallbacks->addUpdateCallback("clearcoat", clearCoatCB);
		setUpdateCallback(_techniqueCallbacks);
	}
	AdvancedMaterial::AdvancedMaterial():ExtensionedMaterial()
	{
		initTechniqueCallback();
		setMaterialFile("materials/advanced.glsl");
	}

	void AdvancedMaterial::addUpdateCallback(const std::string& key, osg::StateAttributeCallback* f)
	{
		_techniqueCallbacks->addUpdateCallback(key, f);
	}
}