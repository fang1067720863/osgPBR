#pragma import_defines(OE_ENABLE_Metal_MAP)
#pragma import_defines(OE_ENABLE_Roughness_MAP)
#pragma import_defines(OE_ENABLE_Roughness_MAP)

uniform sampler2D roughnessMap;
uniform sampler2D metalMap;
uniform sampler2D aoMap;

oe_texcoord = abs(oe_texcoord);

#ifdef OE_ENABLE_Metal_MAP
    vec3 metal =  texture(metalMap, oe_texcoord).rgb;
    metallic *= metal.x;
#endif

#ifdef OE_ENABLE_Roughness_MAP
    vec3 rough =  texture(roughnessMap, oe_texcoord).rgb;
    roughness *= rough.x;
#endif

#ifdef OE_ENABLE_AO_MAP
    vec3 aoFromMap = texture(aoMap, vec3(oe_texcoord,0.0)).rgb;
    ao = aoFromMap.r;
   // return;
#endif