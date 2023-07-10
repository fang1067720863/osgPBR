
const float noiseSpeed = 0.4;

uniform float osg_FrameTime;
uniform sampler2D water_N;
uniform sampler2D water_M;
uniform sampler2D perlin_Noise;

// calc normal

	// large wave
	float time = sin(oe_pos.x/150.f + osg_FrameTime * noiseSpeed);
	vec2 speed = vec2(-0.06f, -0.04f);
	vec2 uv_sin = vec2(0.18f,0.15f) + time * speed;

	vec3 baseNormal = texture(water_N, uv_normal).rgb;
	vec3 additionalNormal = texture(water_N, uv_sin).rgb * vec3(-1,-1,1);

	vec2 pixelNomal = vec3(1.0f,0.0f,0.0f);
	vec3 worldNormal = oe_normal;
	vec3 largeNormal = normalize(mix(baseNormal, additionalNormal, pow(1 - dot(pixelNomal, worldNormal), 2)))

	// small wave
	vec2 uv_large_scale_tiling = vec2(1.0f) * 0.05;
	float noise = texture(perlin_Noise, uv_large_scale_tiling).r;
	float pixelDepthWithNoise = mix(-2000.f, 2000.f, noise);

	// 求解当前像素的深度
	float pixelDepth = length(oe_posView);
	float gradient = clamp((pixelDepth + pixelDepthWithNoise - 1000.0f)/2000.0f,0.0,1.0f);

	vec3 smallNormal = mix(uv_normal, vec3(0,0,1), gradient);
	//BlendAngleCorrectedNormals
	vec3 finalNormal = normalize(mix(largeNormal, smallNormal, pow(1 - dot(pixelNomal, worldNormal), 2)))
	normal = finalNormal;

// calc diffuseColor
	float uv_color = vec2(0.75f) + osg_FrameTime *vec2(-0.07f);
	float uv_normal = vec2(0.05f, 0.08f) + osg_FrameTime *vec2(-0.03f, -0.02f);
	float mask = texture(water_M, uv_normal).r * texture(water_M, uv_color).r;
	mask = clamp(mask, 0.0f, 1,0f);

	vec4 waterBaseColor = vec4(0.0336f, 0.055f, 0.0545f, 1.0f);
	vec4 waveCrestColor = vec4(0.433f, 0.61f,0.601f, 1.0f);
	vec4 waterCrestColor = mix(mix(waterBaseColor, waveCrestColor, mask),waterBaseColor, gradient);
	diffuseColor = waterCrestColor;

// calc roughness
	roughness = 0.1f;

