
#if defined( USE_ENVMAP )

	vec3 getIBLIrradiance( const in vec3 normal ) {



		#if defined( ENVMAP_TYPE_CUBE_UV )

			vec3 worldNormal = inverseTransformDirection( normal, viewMatrix );

			vec4 envMapColor = textureCubeUV( envMap, worldNormal, 1.0 );

			return PI * envMapColor.rgb * envMapIntensity;

		#else

			return vec3( 0.0 );

		#endif

	}
        // 法线在环境贴图上采样
        // 可在表面半球上采样求卷积求平均 得到IBL贴图  最好生成离线的
        // 模型再在IBL贴图上采样 得到IBL环境光强度
        // _CUBE_MAP, irradianceMap

// convolution
//     void main()
// {		
// 	// The world vector acts as the normal of a tangent surface
//     // from the origin, aligned to WorldPos. Given this normal, calculate all
//     // incoming radiance of the environment. The result of this radiance
//     // is the radiance of light coming from -Normal direction, which is what
//     // we use in the PBR shader to sample irradiance.
//     vec3 N = normalize(WorldPos);

//     vec3 irradiance = vec3(0.0);   
    
//     // tangent space calculation from origin point
//     vec3 up    = vec3(0.0, 1.0, 0.0);
//     vec3 right = normalize(cross(up, N));
//     up         = normalize(cross(N, right));
       
//     float sampleDelta = 0.025;
//     float nrSamples = 0.0;
//     for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
//     {
//         for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
//         {
//             // spherical to cartesian (in tangent space)
//             vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
//             // tangent space to world
//             vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

//             irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
//             nrSamples++;
//         }
//     }
//     irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    // 镜面反射环境光强还需要额外两张  都可以预生成
    //brdfLUT  texture2D
    //prefilteredMap textureCube 需要生成mipmap
	vec3 getIBLRadiance( const in vec3 viewDir, const in vec3 normal, const in float roughness ) {

		#if defined( ENVMAP_TYPE_CUBE_UV )

			vec3 reflectVec = reflect( - viewDir, normal );

			// Mixing the reflection with the normal is more accurate and keeps rough objects from gathering light from behind their tangent plane.
			reflectVec = normalize( mix( reflectVec, normal, roughness * roughness) );

			reflectVec = inverseTransformDirection( reflectVec, viewMatrix );

			vec4 envMapColor = textureCubeUV( envMap, reflectVec, roughness );

			return envMapColor.rgb * envMapIntensity;

		#else

			return vec3( 0.0 );

		#endif

	}

#endif
`;


    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;
    
    vec3 color = ambient + Lo;