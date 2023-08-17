#pragma once
#include "PbrMaterial.h"

namespace osgEarth
{

class OE_MATERIAL_PULGIN AdvancedMaterial : public StandardPBRMaterial
{
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
	PROPERTY_DEFAULT(float, IOR, 0.1f)

	PROPERTY_DEFAULT(float, Clearcoat, 0.1f)
	PROPERTY_DEFAULT(float, ClearcoatRoughness, 0.1f)
	PROPERTY_DEFAULT(Vec3f, ClearcoatF0, Vec3(0.0f, 0.0f, 0.0f))
	PROPERTY_DEFAULT(float, ClearcoatF90, 0.1f)

	PROPERTY_DEFAULT(float, Iridescence, 0.1f)
	PROPERTY_DEFAULT(float, IridescenceIOR, 0.1f)
	PROPERTY_DEFAULT(float, IridescenceThickness, 0.1f)
	PROPERTY_DEFAULT(Vec3f, iridescenceFresnel, Vec3(0.0f, 0.0f, 0.0f))
	PROPERTY_DEFAULT(float, iridescenceF0, 0.1f)

	PROPERTY_DEFAULT(float, SheenColor, 0.1f)
	PROPERTY_DEFAULT(float, SheenRoughness, 0.1f)

	PROPERTY_DEFAULT(float, Transmission, 0.1f)
	PROPERTY_DEFAULT(float, TransmissionAlpha, 0.1f)
	PROPERTY_DEFAULT(float, Thickness, 0.1f)
	PROPERTY_DEFAULT(float, AttenuationDistance, 0.1f)
	PROPERTY_DEFAULT(Vec3f, AttenuationColor, Vec3(0.0f, 0.0f, 0.0f))

protected:
	static constexpr const char* IOR = "IOR";
	static constexpr const char* CLEARCOAT = "CLEARCOAT";
	static constexpr const char* IRIDESCENCE = "IRIDESCENCE";
	static constexpr const char* SHEEN = "SHEEN";
	static constexpr const char* TRANSMISSION = "TRANSMISSION";

	static constexpr const char* SHADER_MODEL = "PRINCIPLE_BSDF";
};
} // namespace osgEarth