
#define OE_ENABLE_NORMAL_MAP OE_ENABLE_AO_MAP OE_ENABLE_DIFFUSE_MAP OE_ENABLE_SPECGLOSSY_MAP OE_ENABLE_EMISSIVE_MAP

struct pbr_material_input{
    vec3 diffuseFactor;
    vec3 specularFactor;
    float ao;
    vec3 emissive;
}
#ifdef OE_ENABLE_DIFFUSE_MAP
    diffuseColor = texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_DIFFUSE_MAP)).rgb;
    diffuseColor = diffuseFactor * diffuseColor;
#endif

#ifdef OE_ENABLE_SPECGLOSSY_MAP
    vec3 specularColor =  texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_SPECGLOSSY_MAP)).rgb;
    specularColor = specularFactor * specularColor;
#endif

#ifdef OE_ENABLE_NORMAL_MAP

    vec3 tangent = vec3(1.0,0.0,0.0);
    vec3 tangentNormal = texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_NORMAL_MAP)).rgb;  // normal in tangent space
    vec3 normal = tangentNormal;
#endif

#ifdef OE_ENABLE_AO_MAP
    float aoFromMap = texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_AO_MAP)).r;  // normal in tangent space
    ao = mix(aoFromMap, 1, ao);
#endif

    #ifdef OE_ENABLE_EMISSIVE_MAP
    vec3 emissiveFromMap = texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_EMISSIVE_MAP)).rgb;  // normal in tangent space
    emissive *= emissiveFromMap;
#endif
