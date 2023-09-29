uniform sampler2D sheenColorMap;
uniform sampler2D sheenRoughnessMap;
uniform sampler2D clearcoatMap;
uniform sampler2D clearcoatRoughnessMap;
uniform mat4 osg_ViewMatrix;
uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ProjectionMatrix;

#ifdef USE_SHEEN

	material.sheenColor = oe_pbr.sheenColor;
    material.sheenRoughness = oe_pbr.sheenRoughness;
	#ifdef USE_SHEENCOLORMAP
		material.sheenColor *= texture( sheenColorMap, oe_texcoord ).rgb;
	#endif

	material.sheenRoughness = clamp(material.sheenRoughness, 0.07, 1.0 );
	#ifdef USE_SHEENROUGHNESSMAP
		material.sheenRoughness *= texture( sheenRoughnessMap, oe_texcoord ).a;
	#endif

#endif



// vec3 dxy = max( abs( dFdx( geometryNormal ) ), abs( dFdy( geometryNormal ) ) );
// float geometryRoughness = max( max( dxy.x, dxy.y ), dxy.z );
// 0.0525 corresponds to the base mip of a 256 cubemap.

#ifdef USE_CLEARCOAT
    
	material.clearcoat = oe_pbr.clearcoat;
	material.clearcoatF0 = vec3( 0.04 );
	material.clearcoatF90 = 1.0;
    material.clearcoat = saturate( material.clearcoat );
    float clearcoatRoughness = oe_pbr.clearcoatRoughness;
    material.clearcoatRoughness = max( clearcoatRoughness, 0.0525 );
    material.clearcoatRoughness = min( material.clearcoatRoughness, 1.0 );
    geometry.clearcoatNormal = geometry.normal;
	
	#ifdef USE_CLEARCOATMAP
		material.clearcoat *= texture( clearcoatMap, oe_texcoord ).x;
	#endif

	#ifdef USE_CLEARCOAT_ROUGHNESSMAP
		//material.clearcoatRoughness *= texture( clearcoatRoughnessMap, oe_texcoord ).y;
	#endif

	material.clearcoat = saturate( material.clearcoat ); // Burley clearcoat model
	material.clearcoatRoughness = max( material.clearcoatRoughness, 0.0525 );
	// material.clearcoatRoughness += geometryRoughness;
	material.clearcoatRoughness = min( material.clearcoatRoughness, 1.0 );

#endif


#ifdef USE_TRANSMISSION

	material.transmission = oe_pbr.transmission;
	material.transmissionAlpha = oe_pbr.transmissionAlpha;
	material.thickness = oe_pbr.thickness;
	material.attenuationDistance = oe_pbr.attenuationDistance;
	material.attenuationColor = oe_pbr.attenuationColor;
	material.ior = 0.5f;


	#ifdef USE_TRANSMISSIONMAP

		material.transmission *= texture( transmissionMap, oe_texcoord ).r;

	#endif

	#ifdef USE_THICKNESSMAP

		material.thickness *= texture( thicknessMap, oe_texcoord ).g;

	#endif

	mat4 modelMatrix = osg_ViewMatrixInverse * osg_ModelViewMatrix;
	vec3 pos = (modelMatrix * vec4(oe_pos,1.0f)).xyz;
    vec3 cameraPosition = osg_ViewMatrixInverse[3].xyz;
    vec3 viewInWC = normalize( cameraPosition - pos);
	vec3 normalInWC = (osg_ViewMatrixInverse * vec4(oe_normal,1.0)).xyz;
    
	

	vec4 transmission = getIBLVolumeRefraction(
		oe_normal, viewInWC, material.roughnessFactor, material.baseColorFactor.xyz, vec3(1.0,1.0,1.0), material.metallicFactor,
		pos, modelMatrix, osg_ViewMatrix, osg_ProjectionMatrix, material.ior, material.thickness,
		material.attenuationColor, material.attenuationDistance, oe_probe.pos, translucentMap, oe_texcoord);

	//material.transmissionAlpha = mix( material.transmissionAlpha, transmission.a, material.transmission );

	reflectedLight.backLight = transmission.rgb;
   

#endif