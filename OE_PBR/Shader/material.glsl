
// struct material_output
// {
//     vec3  diffuseColor;
//     float  metallic;
//     float roughness;
//     float normal;
//     float ao;
//     float emissive;
// }output;
#ifdef cascade
    #ifdef OE_ENABLE_BASECOLOR_MAP
        diffuseColor = texture(pbrMaps, vec3(abs(oe_texcoord),OE_ENABLE_BASECOLOR_MAP)).rgb;
    #endif

    #ifdef OE_ENABLE_MR_MAP
        vec3 tmp =  texture(pbrMaps, vec3(abs(oe_texcoord),OE_ENABLE_MR_MAP)).rgb;
        metallic = metallicFactor * tmp.x;
        roughness = roughnessFactor * tmp.y;
    #endif

    #ifdef OE_ENABLE_NORMAL_MAP

        vec3 tangent = vec3(1.0,0.0,0.0);
        vec3 tangentNormal = texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_NORMAL_MAP)).rgb;  // normal in tangent space
        vec3 normal = tangentNormal;
    #endif

    #ifdef OE_ENABLE_AO_MAP
        float aoFromMap = texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_AO_MAP)).r;  // normal in tangent space
        ao *= aoFromMap;
    #endif

        #ifdef OE_ENABLE_EMISSIVE_MAP
        vec3 emissiveFromMap = texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_EMISSIVE_MAP)).rgb;  // normal in tangent space
        emissive *= emissiveFromMap;
    #endif
#endif