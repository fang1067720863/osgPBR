float saturate(float v)
{
    return max(v,0.0f);
}

vec4 SRGBtoLINEAR(vec4 srgbIn)
{
    vec3 linOut = pow(srgbIn.xyz, vec3(2.2f));
    return vec4(linOut, srgbIn.w);
}

vec4 LINEARtoSRGB(vec4 srgbIn)
{
    vec3 linOut = pow(srgbIn.xyz, vec3(1.0 / 2.2f));
    return vec4(linOut, srgbIn.w);
}


vec3 fresnel_GGX(float VdotH, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(1.0 - VdotH, 5);
}

vec3 fresnel_Epic(float VdotH, vec3 f0)
{
    return f0 + (1 - f0) * exp2((-5.55473 * VdotH - 6.98316) * VdotH);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   


float microfacetDistribution(float NdotH, float roughness)
{
    float roughness2 = roughness * roughness;
    float f = NdotH * NdotH * (roughness2 - 1) + 1.0;
    return roughness2 / (PI * f * f);
}

float geometricOcclusion(float NdotL, float NdotV, float roughness)
{
    // End result of remapping:
    float k = pow(roughness + 1, 2) / 8.0f;
    float attenuationV = NdotV / (NdotV * (1 - k) + k);
    float attenuationL = NdotL / (NdotL * (1 - k) + k);
    return attenuationL * attenuationV;
}



vec3 CookTorranceSpecular(float VdotH, float NdotH, float NdotL, float NdotV,
    float roughness, float metalness, vec3 f0, out vec3 kS)
{

    float D = microfacetDistribution(NdotH, roughness);
    vec3 F = fresnel_Epic(VdotH, f0);
    float G = geometricOcclusion(NdotL, NdotV, roughness);
    return (D * F * G) / (4 * max(NdotV * NdotL, 0.01f));
}



vec2 DFGApprox( const in vec3 normal, const in vec3 viewDir, const in float roughness ) {

	float dotNV = saturate( dot( normal, viewDir ) );
	const vec4 c0 = vec4( - 1, - 0.0275, - 0.572, 0.022 );
	const vec4 c1 = vec4( 1, 0.0425, 1.04, - 0.04 );
	vec4 r = roughness * c0 + c1;
	float a004 = min( r.x * r.x, exp2( - 9.28 * dotNV ) ) * r.x + r.y;
	vec2 fab = vec2( - 1.04, 1.04 ) * a004 + r.zw;
	return fab;

}

vec3 EnvironmentBRDF( const in vec3 normal, const in vec3 viewDir, const in vec3 specularColor, const in float specularF90, const in float roughness ) 
{
	vec2 fab = DFGApprox( normal, viewDir, roughness );
	return specularColor * fab.x + specularF90 * fab.y;
}

vec3 BRDF_Diffuse_Lambert(vec3 albedo)
{
    return albedo / PI;
}

vec3 BRDF_Diffuse_Disney(float VdotH, float NdotL, float NdotV,
    float roughness, vec3 diffuseColor)
{
    float Fd90 = 0.5 + 2.0 * roughness * VdotH * VdotH;
    vec3 f0 = vec3(0.1f, 0.1f, 0.1f);
    vec3 invF0 = vec3(1.0, 1.0, 1.0) - f0;
    float dim = min(invF0.r, min(invF0.g, invF0.b));
    float result = ((1.0 + (Fd90 - 1.0) * pow(1.0 - NdotL, 5.0)) * (1.0 + (Fd90 - 1.0) * pow(1.0 - NdotV, 5.0))) * dim;
    return diffuseColor * result;
}

vec3 BRDF_Diffuse_Burley(float VdotH, float NdotL, float NdotV,
    float roughness, vec3 diffuseColor)
{
    float energyBias = mix(roughness, 0.0, 0.5);            // 0.5
    float energyFactor = mix(roughness, 1.0, 1.0 / 1.51);   // 1/3.1415
    float fd90 = energyBias + 2.0 * VdotH * VdotH * roughness;
    float f0 = 1.0;
    float lightScatter = f0 + (fd90 - f0) * pow(1.0 - NdotL, 5.0);
    float viewScatter = f0 + (fd90 - f0) * pow(1.0 - NdotV, 5.0);
    float result = lightScatter * viewScatter * energyFactor;
    return diffuseColor * result;
}

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

    vec3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL;
    vec3 specContrib = numerator / max(denominator, 0.001);
    return specContrib;
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

    float V_Neubelt( float dotNV, float dotNL ) 
    {
        // Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
        return saturate( 1.0 / ( 4.0 * ( dotNL + dotNV - dotNL * dotNV ) ) );

    }

    vec3 BRDF_Sheen( const in vec3 lightDir, const in vec3 viewDir, const in vec3 normal, vec3 sheenColor, const in float sheenRoughness )
    {
        vec3 halfDir = normalize( lightDir + viewDir );

        float dotNL = saturate( dot( normal, lightDir ) );
        float dotNV = saturate( dot( normal, viewDir ) );
        float dotNH = saturate( dot( normal, halfDir ) );

        float D = D_Charlie( sheenRoughness, dotNH );
        float V = V_Neubelt( dotNV, dotNL );

        return sheenColor * ( D * V );
    }

    float IBLSheenBRDF( const in vec3 normal, const in vec3 viewDir, const in float roughness ) 
    {

        float dotNV = saturate( dot( normal, viewDir ) );
        float r2 = roughness * roughness;
        float a = roughness < 0.25 ? -339.2 * r2 + 161.4 * roughness - 25.9 : -8.48 * r2 + 14.3 * roughness - 9.95;
        float b = roughness < 0.25 ? 44.0 * r2 - 23.7 * roughness + 3.26 : 1.97 * r2 - 3.27 * roughness + 0.72;
        float DG = exp( a * dotNV + b ) + ( roughness < 0.25 ? 0.0 : 0.1 * ( roughness - 0.25 ) );
        return saturate( DG * RECIPROCAL_PI );

    }

#endif