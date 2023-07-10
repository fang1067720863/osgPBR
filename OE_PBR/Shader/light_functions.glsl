
// https://www.unrealengine.com/blog/physically-based-shading-on-mobile
vec2 DFGApprox( const in vec3 normal, const in vec3 viewDir, const in float roughness ) {

	float dotNV = 0.1;
    //saturate( dot( normal, viewDir ) );

	const vec4 c0 = vec4( - 1, - 0.0275, - 0.572, 0.022 );

	const vec4 c1 = vec4( 1, 0.0425, 1.04, - 0.04 );

	vec4 r = roughness * c0 + c1;

	float a004 = min( r.x * r.x, exp2( - 9.28 * dotNV ) ) * r.x + r.y;

	vec2 fab = vec2( - 1.04, 1.04 ) * a004 + r.zw;

	return fab;

}

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
    uniform sampler2D brdfLUT;
    uniform mat4 osg_ViewMatrixInverse;
    vec3 getIBLIrradiance(vec3 normal)
    {
        // world space 
        vec3 normalWC = (osg_ViewMatrixInverse * vec4(normal, 0.0)).rgb;
        normalWC = normalize(normalWC);
        float envMapIntensity = 0.2;
        #ifdef USE_ENV_CUBE_UV
            vec3 irradiance = texture(irradianceMap, normal).rgb;
        #else
            vec3 irradiance = texture(irradianceMap, sphericalUV(normal)).rgb;
        #endif
        return PI * irradiance * envMapIntensity;
    }

    // uniform mat4 osg_ViewMatrixInverse;
    vec3 getIBLRadiance(vec3 normal, float roughnessFactor, vec3 viewDir)
    {
        const float MAX_REFLECTION_LOD = 4.0;
        float envMapIntensity =  0.2;

        vec3 r = reflect(-viewDir, normal); 

        vec3 F = fresnelSchlickRoughness(max(dot(normal,viewDir), 0.0), vec3(0.04), roughnessFactor);

         vec3 reflectWC = (osg_ViewMatrixInverse * vec4(r, 0.0)).rgb;
        reflectWC = normalize(reflectWC);
    #ifdef USE_ENV_CUBE_UV
        vec3 prefilteredColor = textureLod(prefilterMap, reflectWC,  roughnessFactor * MAX_REFLECTION_LOD).rgb;
    #else
        vec3 prefilteredColor = textureLod(prefilterMap, sphericalUV(reflectWC),  roughnessFactor * MAX_REFLECTION_LOD).rgb;
    #endif
        vec2 brdf  = texture(brdfLUT, vec2(max(dot(normal,viewDir), 0.0), roughnessFactor)).rg;
        vec3 radiance = prefilteredColor * (F * brdf.x + brdf.y);
        return radiance * envMapIntensity;
    }
#endif


struct pbr_Material
{
    vec4  baseColorFactor;
    vec3  emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    float alphaMask;
    float alphaMaskCutoff;
    float aoStrength;
};

struct ReflectedLight
{
	vec3 indirectDiffuse;
	vec3 indirectSpecular;
	vec3 directDiffuse;
	vec3 directSpecular;
};
struct GeometricContext
{
	vec3 normal;
	vec3 viewDir;
};


void RE_IndirectSpecular_Physical(const in vec3 radiance, const in vec3 irradiance,
								const in GeometricContext geometry, const in pbr_Material material, inout ReflectedLight reflectedLight)
{

	// USE_CLEARCOAT USE_SHEEN USE_IRIDESCENCE

	vec3 singleScattering = vec3( 0.0 );
	vec3 multiScattering = vec3( 0.0 );
	vec3 cosineWeightedIrradiance = irradiance * RECIPROCAL_PI;

    vec3 specularColor =vec3(1.0);
	//computeMultiscattering( geometry.normal, geometry.viewDir, material.specularColor, material.specularF90, material.roughness, singleScattering, multiScattering );
    computeMultiscattering( geometry.normal, geometry.viewDir, specularColor, material.metallicFactor, material.roughnessFactor, singleScattering, multiScattering);


	vec3 scattering = singleScattering + multiScattering;
	vec3 diffuse = material.baseColorFactor.rgb * ( 1.0 - max( max( scattering.r, scattering.g ), scattering.b ) );

	reflectedLight.indirectSpecular += radiance * singleScattering;
	reflectedLight.indirectSpecular += multiScattering * cosineWeightedIrradiance;

	reflectedLight.indirectDiffuse += diffuse * cosineWeightedIrradiance;
	return;

}

void RE_IndirectDiffuse_Physical(vec3 irradiance, vec3 diffuseColor, inout ReflectedLight reflectedLight)
{
	reflectedLight.indirectDiffuse =  irradiance * RECIPROCAL_PI * diffuseColor;
	return;
}