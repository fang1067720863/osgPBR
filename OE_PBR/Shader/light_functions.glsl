


#pragma include struct.glsl
// https://www.unrealengine.com/blog/physically-based-shading-on-mobile


vec3 F_Schlick( const in vec3 f0, const in float f90, const in float dotVH ) {

	float fresnel = exp2( ( - 5.55473 * dotVH - 6.98316 ) * dotVH );
	return f0 * ( 1.0 - fresnel ) + ( f90 * fresnel );

} // validated

void computeMultiscattering( const in vec3 normal, const in vec3 viewDir, const in vec3 specularColor, const in float specularF90, \
									const in float roughness, inout vec3 singleScatter, inout vec3 multiScatter ) {

	vec2 fab = DFGApprox( normal, viewDir, roughness );

	vec3 Fr = specularColor;

	vec3 FssEss = Fr * fab.x + specularF90 * fab.y;

	float Ess = fab.x + fab.y;
	float Ems = 1.0 - Ess;

	vec3 Favg = Fr + ( 1.0 - Fr ) * 0.047619; // 1/21
	vec3 Fms = FssEss * Favg / ( 1.0 - Ems * Favg );

	singleScatter += FssEss;
	multiScatter += Fms * Ems;

}

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 sphericalUV(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan; uv += vec2(0.5); return uv;
}

#ifdef USE_ENV_MAP

   
    #ifdef USE_ENV_CUBE_UV
        uniform samplerCube irradianceMap;
        uniform samplerCube prefilterMap;
    #else
        uniform sampler2D irradianceMap;
        uniform sampler2D prefilterMap;
    #endif
   
    uniform mat4 osg_ViewMatrixInverse;
    vec3 getIBLIrradiance(vec3 normal)
    {
        // world space 
        vec3 normalWC = (osg_ViewMatrixInverse * vec4(normal, 0.0)).rgb;
        normalWC = normalize(normalWC);
        float envMapIntensity = 1.0;
        #ifdef USE_ENV_CUBE_UV
            vec3 irradiance = texture(irradianceMap, normal).rgb;
        #else
            vec3 irradiance = texture(irradianceMap, sphericalUV(normal)).rgb;
        #endif
        return PI * irradiance * envMapIntensity;
    }

    float roughToMip(float _uroughness, float mipLevel)
    {
        float _umip = 0.0;
        if ((_uroughness >= 0.80000001))
        {
            (_umip = ((((1.0 - _uroughness) * 1.0) / 0.19999999) + -2.0));
        }
        else
        {
            if ((_uroughness >= 0.40000001))
            {
            (_umip = ((((0.80000001 - _uroughness) * 3.0) / 0.40000001) + -1.0));
            }
            else
            {
            if ((_uroughness >= 0.30500001))
            {
                (_umip = ((((0.40000001 - _uroughness) * 1.0) / 0.094999999) + 2.0));
            }
            else
            {
                if ((_uroughness >= 0.20999999))
                {
                (_umip = ((((0.30500001 - _uroughness) * 1.0) / 0.095000014) + 3.0));
                }
                else
                {
                (_umip = (-2.0 * log2((1.16 * _uroughness))));
                }
            }
            }
        }
        return mipLevel -1 - (_umip+2) * mipLevel/11;

    }

    uniform float MAX_REFLECTION_LOD;
    vec3 getIBLRadiance(vec3 normal, float roughnessFactor, vec3 viewDir)
    {
        //const float MAX_REFLECTION_LOD = 9.0;
        float envMapIntensity = 1.0;

        vec3 r = reflect(-viewDir, normal); 
        float lod = MAX_REFLECTION_LOD;
        

       

         vec3 reflectWC = (osg_ViewMatrixInverse * vec4(r, 0.0)).rgb;
        reflectWC = normalize(reflectWC);
        float mip = roughToMip(roughnessFactor, MAX_REFLECTION_LOD);
        //return vec3(mip/lod);
    #ifdef USE_ENV_CUBE_UV
        vec3 prefilteredColor = textureLod(prefilterMap, reflectWC,  mip).rgb;
    #else
        vec3 prefilteredColor = textureLod(prefilterMap, sphericalUV(reflectWC),  mip).rgb;
    #endif
         //vec3 F = fresnelSchlickRoughness(max(dot(normal,viewDir), 0.0), vec3(0.04), roughnessFactor);
        // vec2 brdf  = texture(brdfLUT, vec2(max(dot(normal,viewDir), 0.0), roughnessFactor)).rg;
        vec3 radiance = prefilteredColor;
        // * (F * brdf.x + brdf.y);
        return radiance * envMapIntensity;
    }
#endif









void RE_Direct_Physical( const in osg_LightSourceParameters directLight, const in GeometricContext geometry, const in vec3 f0, const in pbr_Material material, inout ReflectedLight reflectedLight) {

    vec3 lightDir = normalize(-directLight.spotDirection.xyz);
	float dotNL = max(dot(geometry.normal, lightDir),0.0f);
	vec3 irradiance = dotNL * vec3(directLight.diffuse);
    irradiance *= directLight.spotExponent;

    #ifdef USE_SHEEN
		vec3 sheenSpecular = irradiance * BRDF_Sheen( lightDir, geometry.viewDir, geometry.normal, material.sheenColor, material.sheenRoughness );
        reflectedLight.sheenSpecular += sheenSpecular;
	#endif

    #ifdef USE_CLEARCOAT
        vec3 clearcoatNormal = vec3(1.0,0.0,0.0);
       	float dotNLcc = saturate( dot( clearcoatNormal, lightDir ) );
		vec3 ccIrradiance = dotNLcc * directLight.diffuse.rgb;
        
		reflectedLight.clearcoatSpecular += ccIrradiance * BRDF_GGX( lightDir, geometry.viewDir, clearcoatNormal,material.clearcoatRoughness, material.clearcoatF90,material.clearcoatF0 );
    #endif


	reflectedLight.directSpecular += irradiance * BRDF_GGX(lightDir, geometry.viewDir, geometry.normal, material.roughnessFactor, material.metallicFactor, f0 );
	reflectedLight.directDiffuse += irradiance * BRDF_Diffuse_Lambert( material.baseColorFactor.rgb );
}

uniform sampler2D brdfLUT;
void RE_IndirectSpecular_Physical(const in vec3 radiance, const in vec3 irradiance, const in vec3 clearcoatRadiance,const in vec3 f0,
								const in GeometricContext geometry, const in pbr_Material material, inout ReflectedLight reflectedLight)
{

	#ifdef USE_SHEEN

		//reflectedLight.sheenSpecular += irradiance * material.sheenColor * IBLSheenBRDF( geometry.normal, geometry.viewDir, material.sheenRoughness );

	#endif

    #ifdef USE_CLEARCOAT
        reflectedLight.clearcoatSpecular += clearcoatRadiance * EnvironmentBRDF(geometry.clearcoatNormal, geometry.viewDir, material.clearcoatF0, material.clearcoatF90, material.clearcoatRoughness);
    #endif

	vec3 singleScattering = vec3( 0.0 );
	vec3 multiScattering = vec3( 0.0 );
	vec3 cosineWeightedIrradiance = irradiance * RECIPROCAL_PI;

    vec3 specularColor =vec3(1.0);
    computeMultiscattering( geometry.normal, geometry.viewDir, specularColor, material.metallicFactor, material.roughnessFactor, singleScattering, multiScattering);
     float NdotV = max(dot( geometry.normal, geometry.viewDir), 0.0);
    vec3 F = fresnelSchlickRoughness(NdotV, f0, material.roughnessFactor);
    vec2 brdf  = texture(brdfLUT, vec2(NdotV, material.roughnessFactor)).rg;
    singleScattering = (F * brdf.x + brdf.y* vec3(material.metallicFactor));


	vec3 scattering = singleScattering + multiScattering;
	vec3 diffuse = material.baseColorFactor.rgb * ( 1.0 - max( max( scattering.r, scattering.g ), scattering.b ) ); 

	reflectedLight.indirectSpecular += singleScattering * radiance; 
   
   
	reflectedLight.indirectSpecular += multiScattering * cosineWeightedIrradiance;

	reflectedLight.indirectDiffuse += diffuse * cosineWeightedIrradiance;
	return;

}

void RE_IndirectDiffuse_Physical(vec3 irradiance, vec3 diffuseColor, inout ReflectedLight reflectedLight)
{
	reflectedLight.indirectDiffuse =  irradiance * RECIPROCAL_PI * diffuseColor;
	return;
}