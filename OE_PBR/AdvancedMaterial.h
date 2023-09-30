#pragma once
#include "PbrMaterial.h"
#include<vector>
#include<osg/Texture2D>

namespace osgEarth
{
using F = std::function<void(osg::StateAttribute* attr, osg::NodeVisitor* nv)>;
struct SAttrCallback : public osg::StateAttributeCallback
{
	SAttrCallback()  {}
	void operator()(osg::StateAttribute* attr, osg::NodeVisitor* nv) {
		for (auto& f : _funList)
		{
			f(attr, nv);
		}
		for (auto& cb : _subCallbacks)
		{
			cb->operator()(attr, nv);
		}
	}
	std::vector<F> _funList;
	std::vector<osg::ref_ptr<osg::StateAttributeCallback>> _subCallbacks;
	std::vector<std::string> _techniqueKeys;
public:

	bool exist(const std::string& key)
	{
		for (const auto& _key : _techniqueKeys)
		{
			if (_key == key)
			{
				return true;
			}
		}
		return false;
	}
	void addUpdateCallback(const std::string& key, F&& f) {
		if (exist(key)) return;
		printf("<<<<<<<<<<<<<<<<<<<<<<<<<adProbe%s\n", key.c_str());
		_funList.push_back(std::move(f)); }
	void addUpdateCallback(const std::string& key, osg::StateAttributeCallback* f) {
		if (exist(key)) return; 
		printf("<<<<<<<<<<<<<<<<<<<<<<<<<adProbe%s\n", key.c_str());
		_subCallbacks.push_back(f);
	}
};

static const unsigned int TRANSLUCENT_MASK = 0x5; // 0100

//1.0£¨air£©¡¢1.33£¨water£©¡¢1.52£¨glass£©ÒÔ¼°2.42£¨diamo	nd£©
class OE_MATERIAL_PULGIN AdvancedMaterial : public ExtensionedMaterial
{
public:
	AdvancedMaterial();

	void addUpdateCallback(const std::string& key, osg::StateAttributeCallback* f);
	void addUpdateCallback(const std::string& key, F&& f);

	enum TextureEnum
	{
		CLEARCOATMAP = 6,
		CLEARCOAT_ROUGHNESSMAP = 7,
		IRIDESCENCEMAP = 8,
		IRIDESCENCE_THICKNESSMAP = 9,
		SHEENCOLORMAP = 10,
		SHEENROUGHNESSMAP = 11,
		TRANSMISSIONMAP = 12,
		THICKNESSMAP = 13,
	};

	const std::unordered_map<TextureEnum, std::string> textureDefines = {
	    {CLEARCOATMAP, "USE_CLEARCOATMAP"},
	    {CLEARCOAT_ROUGHNESSMAP, "USE_CLEARCOAT_ROUGHNESSMAP"},
	    {IRIDESCENCEMAP, "USE_IRIDESCENCEMAP"},
	    {IRIDESCENCE_THICKNESSMAP, "USE_IRIDESCENCE_THICKNESSMAP"},
	    {SHEENCOLORMAP, "USE_SHEENCOLORMAP"},
	    {SHEENROUGHNESSMAP, "USE_SHEENROUGHNESSMAP"},
	    {TRANSMISSIONMAP, "USE_TRANSMISSIONMAP"},
	    {THICKNESSMAP, "USE_THICKNESSMAP"},
	};
	PROPERTY_DEFAULT(float, IOR, 1.2f)

	PROPERTY_DEFAULT(bool, UseClearcoat, false)
	PROPERTY_DEFAULT(float, Clearcoat, 0.5f)
	PROPERTY_DEFAULT(float, ClearcoatRoughness, 0.5f)
	PROPERTY_DEFAULT(osg::ref_ptr<osg::Texture2D>, ClearcoatRoughnessMap, 0)
	PROPERTY_DEFAULT(osg::ref_ptr<osg::Texture2D>, ClearcoatNormalMap, 0)
	PROPERTY_DEFAULT(Vec2f, ClearcoatNormalScale, osg::Vec2f(0.1f,0.1f))
	/*PROPERTY_DEFAULT(Vec3f, ClearcoatF0, Vec3(0.04f, 0.04f, 0.04f))
	PROPERTY_DEFAULT(float, ClearcoatF90, 1.0f)*/

	PROPERTY_DEFAULT(float, Iridescence, 0.1f)
	PROPERTY_DEFAULT(float, IridescenceIOR, 0.1f)
	PROPERTY_DEFAULT(float, IridescenceThickness, 0.1f)
	PROPERTY_DEFAULT(Vec3f, IridescenceFresnel, Vec3(0.0f, 0.0f, 0.0f))
	PROPERTY_DEFAULT(float, IridescenceF0, 0.1f)

	PROPERTY_DEFAULT(bool,  UseSheen,false)
	PROPERTY_DEFAULT(Vec3f, SheenColor, Vec3(0.0f, 0.5f, 0.0f))
	PROPERTY_DEFAULT(float, SheenRoughness, 0.1f)
	PROPERTY_DEFAULT(osg::ref_ptr<osg::Texture2D>, SheenColorMap, 0)
	PROPERTY_DEFAULT(osg::ref_ptr<osg::Texture2D>, SheenRoughnessMap, 0)

	PROPERTY_DEFAULT(bool, UseTransmission, false)
	PROPERTY_DEFAULT(float, Transmission, 0.1f)
	PROPERTY_DEFAULT(float, TransmissionAlpha, 0.1f)
	PROPERTY_DEFAULT(float, Thickness, 0.1f)
	PROPERTY_DEFAULT(float, AttenuationDistance, 0.1f)
	PROPERTY_DEFAULT(Vec3f, AttenuationColor, Vec3(0.5f, 0.5f, 0.5f))
	PROPERTY_DEFAULT(osg::ref_ptr<osg::Texture2D>, ThicknessMap, 0)
public:

	void setMaterialImage(AdvancedMaterial::TextureEnum mapEnum, osg::Image* image);
	void setMaterialImage(StandardPBRMaterial::TextureEnum mapEnum, osg::Image* image);
	void setMaterialImage(StandardPBRMaterial::TextureEnum mapEnum, const std::string& imageUrl);
protected:

	void initTechniqueCallback();
	

	osg::ref_ptr<SAttrCallback> _techniqueCallbacks;
	static constexpr const char* IOR = "IOR";
	static constexpr const char* CLEARCOAT = "CLEARCOAT";
	static constexpr const char* IRIDESCENCE = "IRIDESCENCE";
	static constexpr const char* SHEEN = "SHEEN";
	static constexpr const char* TRANSMISSION = "TRANSMISSION";

	static constexpr const char* SHADER_MODEL = "PRINCIPLE_BSDF";


	static const std::string USE_IOR;
	static const std::string USE_CLEARCOAT;
	static const std::string USE_IRIDESCENCE;
	static const std::string USE_SHEEN;
	static const std::string USE_TRANSMISSION;
};
} // namespace osgEarth