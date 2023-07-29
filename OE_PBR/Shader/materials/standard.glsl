
//uniform float osg_FrameTime;



#ifdef cascade
    oe_texcoord = abs(oe_texcoord);
    #ifdef OE_ENABLE_BASECOLOR_MAP
        diffuseColor = texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_BASECOLOR_MAP)).rgb;
      //  color.rgb = diffuseColor;
      //  return;
    #endif

    #ifdef OE_ENABLE_MR_MAP
        vec3 tmp =  texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_MR_MAP)).rgb;
        metallic = tmp.b;
        roughness = tmp.g;
    #endif

    #ifdef OE_ENABLE_NORMAL_MAP

        vec3 tangent = vec3(1.0,0.0,0.0) * gl_NormalMatrix;
        vec3 mapN = texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_NORMAL_MAP)).rgb;  // normal in tangent space
        normal = getNormal(normal, mapN);

    #endif

    // #ifdef OE_ENABLE_AO_MAP
    //     vec3 aoFromMap = texture(pbrMaps, vec3(oe_texcoord,0.0)).rgb;
    //     //color.rgb = aoFromMap;
    //    // return;
    // #endif

        #ifdef OE_ENABLE_EMISSIVE_MAP
        vec3 emissiveFromMap = texture(pbrMaps, vec3(oe_texcoord,OE_ENABLE_EMISSIVE_MAP)).rgb;  
        emissive *= emissiveFromMap;
    #endif
#endif