

vec3 fresnel_GGX(float VdotH, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(1.0 - VdotH, 5);
}

vec3 fresnel_Epic(float VdotH, vec3 f0)
{
    return f0 + (1 - f0) * exp2((-5.55473 * VdotH - 6.98316) * VdotH);
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

vec3 CookTorranceSpecular(float VdotH, float NdotH, float NdotL, float NdotV,
    float roughness, float metalness, vec3 f0, out vec3 kS)
{

    float D = microfacetDistribution(NdotH, roughness);
    vec3 F = fresnel_Epic(VdotH, f0);
    float G = geometricOcclusion(NdotL, NdotV, roughness);
    return (D * F * G) / (4 * max(NdotV * NdotL, 0.01f));
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


// vec3 F_Schlick( const in vec3 f0, const in float f90, const in float dotVH ) {

// 	float fresnel = exp2( ( - 5.55473 * dotVH - 6.98316 ) * dotVH );

// 	return f0 * ( 1.0 - fresnel ) + ( f90 * fresnel );

// } // validated

// float F_Schlick( const in float f0, const in float f90, const in float dotVH ) {

// 	float fresnel = exp2( ( - 5.55473 * dotVH - 6.98316 ) * dotVH );

// 	return f0 * ( 1.0 - fresnel ) + ( f90 * fresnel );

// } // validated

// vec3 Schlick_to_F0( const in vec3 f, const in float f90, const in float dotVH ) {
//     float x = clamp( 1.0 - dotVH, 0.0, 1.0 );
//     float x2 = x * x;
//     float x5 = clamp( x * x2 * x2, 0.0, 0.9999 );

//     return ( f - vec3( f90 ) * x5 ) / ( 1.0 - x5 );
// }


// float V_GGX_SmithCorrelated( const in float alpha, const in float dotNL, const in float dotNV ) {

// 	float a2 = pow2( alpha );

// 	float gv = dotNL * sqrt( a2 + ( 1.0 - a2 ) * pow2( dotNV ) );
// 	float gl = dotNV * sqrt( a2 + ( 1.0 - a2 ) * pow2( dotNL ) );

// 	return 0.5 / max( gv + gl, EPSILON );

// }


// float D_GGX( const in float alpha, const in float dotNH ) {

// 	float a2 = pow2( alpha );

// 	float denom = pow2( dotNH ) * ( a2 - 1.0 ) + 1.0; // avoid alpha = 0 with dotNH = 1

// 	return RECIPROCAL_PI * a2 / pow2( denom );

// }

// // GGX Distribution, Schlick Fresnel, GGX_SmithCorrelated Visibility
// vec3 BRDF_GGX( const in vec3 lightDir, const in vec3 viewDir, const in vec3 normal, const in vec3 f0, const in float f90, const in float roughness ) {

// 	float alpha = pow2( roughness ); // UE4's roughness

// 	vec3 halfDir = normalize( lightDir + viewDir );

// 	float dotNL = saturate( dot( normal, lightDir ) );
// 	float dotNV = saturate( dot( normal, viewDir ) );
// 	float dotNH = saturate( dot( normal, halfDir ) );
// 	float dotVH = saturate( dot( viewDir, halfDir ) );

// 	vec3 F = F_Schlick( f0, f90, dotVH );

// 	float V = V_GGX_SmithCorrelated( alpha, dotNL, dotNV );

// 	float D = D_GGX( alpha, dotNH );

// 	return F * ( V * D );

// }

vec3 BRDF(
         float VdotH,
         float NdotH,
         float NdotL,
         float NdotV,
         float roughness,
         float metalness,
         vec3 f0,
         vec3 diffuseColor,
         vec3 lightColor,
         float ao
        )
{
    float D = microfacetDistribution(NdotH, roughness);
    vec3 F = fresnel_Epic(VdotH, f0);
    float G = geometricOcclusion(NdotL, NdotV, roughness);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;


    vec3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL;
    vec3 specContrib = numerator / max(denominator, 0.001);

    vec3 diffuse = BRDF_Diffuse_Lambert(diffuseColor);
   // vec3 diffuse = BRDF_Diffuse_Burley(VdotH, NdotL, NdotV, roughness, diffuseColor);
    vec3 diffuseContrib = diffuse * kD;
    
    vec3 Lo = NdotL * lightColor * (diffuseContrib + specContrib);


    return Lo;
    
}




vec3 normalInWorldWithoutTangent(vec3 normal, vec3 eyePos,vec2 uv, vec3 normalFromTexture)
{
    // Perturb normal, see http://www.thetenthplanet.de/archives/1180
    vec3 unpackedNormal = normalFromTexture * 2.0f - 1.0f;

    // vec3 q1 = ddx(eyePos);
    // vec3 q2 = ddy(eyePos);
    // vec2 st1 = ddx(uv);
    // vec2 st2 = ddy(uv);

    // vec3 N = normalize(normal);
    // vec3 T = normalize(q1 * st2.y - q2 * st1.y);
    // vec3 B = normalize(cross(N, T));

	// mat3 TBN = mat3(T, B, N);
    // return normalize(TBN*unpackedNormal);
    return normal;

}

vec3 getNormal(vec3 N, vec3 normalFromTexture)
{

    vec3 unpackedNormal = normalFromTexture*2.0 -1.0;  // normal in tangent space
    // vec3 tangent = gl_NormalMatrix * vec3(1.0,0.0,0.0);

    // vec3 N = normalize(normal);
    // vec3 T =  normalize(tangent - N * dot(tangent, N));
    // vec3 B = cross(N, T);
    // mat3 TBN = mat3(T, B, N);
    // return normalize(TBN * unpackedNormal);

    vec3 B = normalize(gl_NormalMatrix * vec3(0,1,0));

    vec3 T = normalize(cross(B, N));

    return normalize(mat3(T, B, N) * unpackedNormal);
	

}

mat3 makeTBN(vec3 N, vec3 p, vec2 uv)
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    // solve the linear system
    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construct a scale-invariant frame 
    float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
    return mat3(T * invmax, B * invmax, N);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   