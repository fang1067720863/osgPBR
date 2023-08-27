


#pragma include struct.glsl
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

vec3 EnvironmentBRDF( const in vec3 normal, const in vec3 viewDir, const in vec3 specularColor, const in float specularF90, const in float roughness ) {

	vec2 fab = DFGApprox( normal, viewDir, roughness );

	return specularColor * fab.x + specularF90 * fab.y;

}

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





vec3 BRDF_GGX(
        vec3 direction,
        vec3 viewDir,
        vec3 normal,
        float roughness,
        float metalness,
        vec3 f0
        )
{
    vec3 h = normalize(direction + viewDir);

    float NdotL = max(dot(normal, direction), 0.0f);
    float VdotH = max(dot(h, viewDir), 0.0f);
    float NdotH = max(dot(normal, h), 0.0f);
    float NdotV = max(dot(normal, viewDir), 0.0f);

    float D = microfacetDistribution(NdotH, roughness);
    vec3 F = fresnel_Epic(VdotH, f0);
    float G = geometricOcclusion(NdotL, NdotV, roughness);

    // vec3 kS = F;
    // vec3 kD = vec3(1.0) - kS;
     // vec3 diffuse = BRDF_Diffuse_Lambert(diffuseColor);
    // vec3 diffuseContrib = diffuse * kD;

    vec3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL;
    vec3 specContrib = numerator / max(denominator, 0.001);
    return specContrib;
    //return vec3(0.0f);

    
}

float saturate(float v)
{
    return max(v,0.0f);
}

#ifdef USE_SHEEN


    float D_Charlie( float roughness, float dotNH ) {

        float alpha = roughness * roughness;

        // Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF"
        float invAlpha = 1.0 / alpha;
        float cos2h = dotNH * dotNH;
        float sin2h = max( 1.0 - cos2h, 0.0078125 ); // 2^(-14/2), so sin2h^2 > 0 in fp16

        return ( 2.0 + invAlpha ) * pow( sin2h, invAlpha * 0.5 ) / ( 2.0 * PI );

    }

    float V_Neubelt( float dotNV, float dotNL ) {

        // Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
        return saturate( 1.0 / ( 4.0 * ( dotNL + dotNV - dotNL * dotNV ) ) );

    }

    vec3 BRDF_Sheen( const in vec3 lightDir, const in vec3 viewDir, const in vec3 normal, vec3 sheenColor, const in float sheenRoughness ) {

        vec3 halfDir = normalize( lightDir + viewDir );

        float dotNL = saturate( dot( normal, lightDir ) );
        float dotNV = saturate( dot( normal, viewDir ) );
        float dotNH = saturate( dot( normal, halfDir ) );

        float D = D_Charlie( sheenRoughness, dotNH );
        float V = V_Neubelt( dotNV, dotNL );

        return sheenColor * ( D * V );

    }

    float IBLSheenBRDF( const in vec3 normal, const in vec3 viewDir, const in float roughness ) {

        float dotNV = saturate( dot( normal, viewDir ) );

        float r2 = roughness * roughness;

        float a = roughness < 0.25 ? -339.2 * r2 + 161.4 * roughness - 25.9 : -8.48 * r2 + 14.3 * roughness - 9.95;

        float b = roughness < 0.25 ? 44.0 * r2 - 23.7 * roughness + 3.26 : 1.97 * r2 - 3.27 * roughness + 0.72;

        float DG = exp( a * dotNV + b ) + ( roughness < 0.25 ? 0.0 : 0.1 * ( roughness - 0.25 ) );

        return saturate( DG * RECIPROCAL_PI );

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
       	float dotNLcc = saturate( dot( geometry.clearcoatNormal, lightDir ) );
		vec3 ccIrradiance = dotNLcc * directLight.diffuse.rgb;
        
		reflectedLight.clearcoatSpecular += ccIrradiance * BRDF_GGX( lightDir, geometry.viewDir, geometry.clearcoatNormal, material.clearcoatF90, material.clearcoatRoughness,material.clearcoatF0 );
    #endif


	reflectedLight.directSpecular += irradiance * BRDF_GGX(lightDir, geometry.viewDir, geometry.normal, material.roughnessFactor, material.metallicFactor, f0 );
	reflectedLight.directDiffuse += irradiance * BRDF_Diffuse_Lambert( material.baseColorFactor.rgb );
}

uniform sampler2D brdfLUT;
void RE_IndirectSpecular_Physical(const in vec3 radiance, const in vec3 irradiance, const in vec3 clearcoatRadiance,const in vec3 f0,
								const in GeometricContext geometry, const in pbr_Material material, inout ReflectedLight reflectedLight)
{

	#ifdef USE_SHEEN

		reflectedLight.sheenSpecular += irradiance * material.sheenColor * IBLSheenBRDF( geometry.normal, geometry.viewDir, material.sheenRoughness );

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
    singleScattering = (F * brdf.x + brdf.y);


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