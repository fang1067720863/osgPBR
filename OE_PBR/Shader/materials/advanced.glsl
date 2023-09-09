uniform sampler2D sheenColorMap;
uniform sampler2D sheenRoughnessMap;
uniform sampler2D clearcoatMap;
uniform sampler2D clearcoatRoughnessMap;

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
		material.clearcoatRoughness *= texture( clearcoatRoughnessMap, oe_texcoord ).y;
	#endif

	material.clearcoat = saturate( material.clearcoat ); // Burley clearcoat model
	material.clearcoatRoughness = max( material.clearcoatRoughness, 0.0525 );
	// material.clearcoatRoughness += geometryRoughness;
	material.clearcoatRoughness = min( material.clearcoatRoughness, 1.0 );

#endif
