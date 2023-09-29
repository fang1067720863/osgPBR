#pragma import_defines(OE_ENABLE_Metal_MAP)
#pragma import_defines(OE_ENABLE_Roughness_MAP)
#pragma import_defines(OE_ENABLE_Roughness_MAP)

uniform sampler2D roughnessMap;
uniform sampler2D metalMap;




#ifdef OE_ENABLE_Metal_MAP
    vec3 metal =  texture(metalMap, oe_texcoord).rgb;
    metallic *= metal.x;
#endif

#ifdef OE_ENABLE_Roughness_MAP
    vec3 rough =  texture(roughnessMap, oe_texcoord).rgb;
    roughness *= rough.x;
#endif

f0 = mix(f0, diffuseColor, vec3(metallic));
material.metallicFactor=metallic;
material.roughnessFactor=min(roughness,0.04f);