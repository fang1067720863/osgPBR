


vec3 getVolumeTransmissionRay( const in vec3 n, const in vec3 v, const in float thickness, const in float ior, const in mat4 modelMatrix ) 
{

	// Direction of refracted light.
	vec3 refractionVector = refract(v, normalize( n ), 1.0 / 1.0 );

	// Compute rotation-independant scaling of the model matrix.
	vec3 modelScale;
	modelScale.x = length( vec3( modelMatrix[ 0 ].xyz ) );
	modelScale.y = length( vec3( modelMatrix[ 1 ].xyz ) );
	modelScale.z = length( vec3( modelMatrix[ 2 ].xyz ) );

	// The thickness is specified in local space.
	return normalize( refractionVector ) * thickness;

}

float applyIorToRoughness( const in float roughness, const in float ior ) 
{

	// Scale roughness with IOR so that an IOR of 1.0 results in no microfacet refraction and
	// an IOR of 1.5 results in the default amount of microfacet refraction.
	return roughness * clamp( ior * 2.0 - 2.0, 0.0, 1.0 );

}

vec4 getTransmissionSample( const in vec3 probeSampleVec, const in float roughness, const in float ior ) 
{

	//float framebufferLod = log2( transmissionSamplerSize.x ) * applyIorToRoughness( roughness, ior );

	// #ifdef texture2DLodEXT

	// 	return texture2DLodEXT( transmissionSamplerMap, fragCoord.xy, framebufferLod );

	// #else

	
	return vec4(1.0f);

}

vec3 applyVolumeAttenuation( const in vec3 radiance, const in float transmissionDistance, const in vec3 attenuationColor, const in float attenuationDistance ) 
{

	if ( attenuationDistance>1000.0f ) {
		return radiance;

	} else {

		vec3 attenuationCoefficient = -log( attenuationColor ) / attenuationDistance;
		vec3 transmittance = exp( - attenuationCoefficient * transmissionDistance ); // Beer's law
		return transmittance * radiance;

	}

}

vec4 getIBLVolumeRefraction( const in vec3 n, const in vec3 v, const in float roughness, const in vec3 diffuseColor,
	const in vec3 specularColor, const in float specularF90, const in vec3 position, const in mat4 modelMatrix,
	const in mat4 viewMatrix, const in mat4 projMatrix, const in float ior, const in float thickness,
	const in vec3 attenuationColor, const in float attenuationDistance, const in vec3 probePos, sampler2D translucentMap, vec2 uv) 
{

	vec3 transmissionRay = getVolumeTransmissionRay( n, v, thickness, ior, modelMatrix );
	vec3 refractedRayExit = position + transmissionRay;

	vec4 ndcPos = projMatrix * viewMatrix * vec4( refractedRayExit, 1.0 );
	vec2 refractionCoords = ndcPos.xy / ndcPos.w;
	refractionCoords += 1.0;
	refractionCoords /= 2.0;

	// vec3 dirToProbe = probePos - position;
	// vec3 probeSampleVec = refractedRayExit - dirToProbe;

	// probeSampleVec = vec3(1.0,0.0,0.0);
	// vec3 off = sin(refractedRayExit.x) * vec3(1.0,1.0,0.0);

	vec4 transmittedLight = texture( translucentMap, refractionCoords, 0 );
	//getTransmissionSample( probeSampleVec, roughness, ior );

	//return transmittedLight;

	//vec4 transmittedLight = vec4(0.5);
	vec3 attenuatedColor = applyVolumeAttenuation( transmittedLight.rgb, length( transmissionRay ), attenuationColor, attenuationDistance );
	vec3 F = EnvironmentBRDF( n, v, specularColor, specularF90, roughness );

	return vec4( ( 1.0 - F ) * attenuatedColor.rgb * diffuseColor, transmittedLight.a );

}

