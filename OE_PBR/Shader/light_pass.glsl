// #if defined( USE_ENVMAP )

// 	vec3 getIBLIrradiance( const in vec3 normal ) {

// 		#if defined( ENVMAP_TYPE_CUBE_UV )

// 			vec3 worldNormal = inverseTransformDirection( normal, viewMatrix );

// 			vec4 envMapColor = textureCubeUV( envMap, worldNormal, 1.0 );

// 			return PI * envMapColor.rgb * envMapIntensity;

// 		#else

// 			return vec3( 0.0 );

// 		#endif

// 	}

// 	vec3 getIBLRadiance( const in vec3 viewDir, const in vec3 normal, const in float roughness ) {

// 		#if defined( ENVMAP_TYPE_CUBE_UV )

// 			vec3 reflectVec = reflect( - viewDir, normal );

// 			// Mixing the reflection with the normal is more accurate and keeps rough objects from gathering light from behind their tangent plane.
// 			reflectVec = normalize( mix( reflectVec, normal, roughness * roughness) );

// 			reflectVec = inverseTransformDirection( reflectVec, viewMatrix );

// 			vec4 envMapColor = textureCubeUV( envMap, reflectVec, roughness );

// 			return envMapColor.rgb * envMapIntensity;

// 		#else

// 			return vec3( 0.0 );

// 		#endif

// 	}

// #endif

// name policy
// light eg. radiance irradiance radianceIBL
// material properties eg. diffuseColor specularColor emissiveColor
// shading output eg. directDiffuse directSpecular indirectDiffuse







// vec3 indirectSpecular = vec3(0.0);
// vec3 indirectDiffuse = vec3(0.0);

// irradiance = getAmbientLightIrradiance();
// irradiance += getLightProbeIrradiance(); 
// irradiance += getLightMapIrradiance();

// irradianceIBL += getIBLIrradiance();
// radianceIBL += getIBLRadiance();

// // indirect shading
// RE_IndirectDiffuse_Physical(irradiance, diffuseColor, reflectedLight);
// RE_IndirectSpecular_Physical(radianceIBL, irradianceIBL);






// #ifdef USE_ENV_MAP

//     vec3 n2 = normalize(normal);
//     vec3 v2 = normalize(-oe_posView);
//     vec3 F = fresnelSchlickRoughness(max(dot(n2,v2), 0.0), vec3(0.04), roughnessFactor);

//     vec3 kS = F;
//     vec3 kD = 1.0 - kS;
//     kD *= 1.0 - metallic;
// #ifdef USE_ENV_CUBE_UV
//     vec3 irradiance = texture(irradianceMap, oe_normal).rgb;
// #else
//     vec3 irradiance = texture(irradianceMap, sphericalUV(oe_normal)).rgb;
// #endif
//     vec3 diffuse      = irradiance * baseColor;
    
//     // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
//     const float MAX_REFLECTION_LOD = 4.0;
// #ifdef USE_ENV_CUBE_UV
//     vec3 prefilteredColor = textureLod(prefilterMap, oe_normal,  roughnessFactor * MAX_REFLECTION_LOD).rgb;
// #else
//     vec3 prefilteredColor = textureLod(prefilterMap, sphericalUV(oe_normal),  roughnessFactor * MAX_REFLECTION_LOD).rgb;
// #endif
//     vec2 brdf  = texture(brdfLUT, vec2(max(dot(oe_normal,v), 0.0), roughnessFactor)).rg;
//     vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

//     ambient = (kD * diffuse + specular) * ao;
    
// #else
//    ambient = osg_LightSource[0].ambient.rgb * diffuseColor * ao;
// #endif
//     color.rgb = Lo + ambient;

