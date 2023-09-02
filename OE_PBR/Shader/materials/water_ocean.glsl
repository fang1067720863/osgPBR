uniform float osg_FrameTime;
uniform sampler2D water_N;
uniform sampler2D water_M;
uniform sampler2D perlin_Noise;


// calc normal
	
	const float noiseSpeed = 0.4;
	// large wave
	float time = sin(oe_pos.x/150.f + osg_FrameTime * noiseSpeed);

	vec2 uv_sin = vec2(0.18f,0.15f) + time * vec2(-0.06f, -0.04f);
	vec2 uv_color = vec2(0.75f) + osg_FrameTime *vec2(-0.07f);
	vec2 uv_normal = vec2(0.05f, 0.08f) + osg_FrameTime *vec2(-0.03f, -0.02f);
	uv_sin = fract(oe_texcoord + uv_sin);
	uv_color = fract(oe_texcoord + uv_color);
	uv_normal = fract(oe_texcoord + uv_normal);

	vec3 baseNormal = texture(water_N, uv_normal).rgb;
	vec3 additionalNormal = texture(water_N, uv_sin).rgb * vec3(-1,1,-1);
	baseNormal = getNormal(normal, baseNormal);
	additionalNormal = getNormal(normal, additionalNormal);

	vec3 pixelNomal = vec3(1.0f,0.0f,0.0f);
	vec3 worldNormal = oe_normal;
	vec3 largeNormal = normalize(mix(baseNormal, additionalNormal, pow(1 - dot(pixelNomal, worldNormal), 2)));

	// small wave
	vec2 uv_large_scale_tiling = vec2(1.0f) * 0.05f;
	float noise = texture(perlin_Noise, uv_large_scale_tiling).r;
	float pixelDepthWithNoise = mix(-2000.f, 2000.f, noise);

	// 
	float pixelDepth = length(oe_posView);
	float gradient = clamp((pixelDepth + pixelDepthWithNoise - 1000.0f)/2000.0f,0.0,1.0f);

	vec3 smallNormal = mix(texture(water_N, uv_color).rgb, vec3(0,0,1), gradient);
	smallNormal = getNormal(normal, smallNormal);
	//BlendAngleCorrectedNormals
	vec3 finalNormal = normalize(mix(largeNormal, smallNormal, pow(1 - dot(pixelNomal, worldNormal), 2)));
	normal = finalNormal;

// calc diffuseColor
	//float gradient = 0.1f;
	
	float mask = texture(water_M, uv_normal).r * texture(water_M, uv_color).r;
	mask = clamp(mask, 0.0f, 1.0f);

	vec4 waterBaseColor = vec4(0.0336f, 0.055f, 0.0545f, 1.0f);
	vec4 waveCrestColor = vec4(0.433f, 0.61f,0.601f, 1.0f);
	vec4 waterCrestColor = mix(mix(waterBaseColor, waveCrestColor, mask),waterBaseColor, gradient);
	diffuseColor = waterCrestColor.rgb;

// calc roughness
	roughness = 0.1f;

#ifdef debug_texture
    color.rgb = normal;
#endif
