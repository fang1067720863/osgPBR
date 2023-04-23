
// #pragma vp_function BRDF, fragment, first
// vp_function is a function which will be execute in specific stage, like fragment_color, can be added or removed dynamically 


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
         float ao,
         vec3 emissive
        )
{
    float D = microfacetDistribution(NdotH, roughness);
    vec3 F = fresnel_Epic(VdotH, f0);
    float G = geometricOcclusion(NdotL, NdotV, roughness);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metalness;


    vec3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL;
    vec3 specContrib = numerator / max(denominator, 0.001);

    vec3 diffuse = BRDF_Diffuse_Burley(VdotH, NdotL, NdotV, roughness, diffuseColor);
    vec3 diffuseContrib = (vec3(1.0f, 1.0f, 1.0f) - F)  * diffuse;
    
    vec3 Lo = NdotL * lightColor * (diffuseContrib + specContrib);

    //Lo *= ao;
    Lo += emissive;

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

vec3 getNormal(vec3 normal, vec3 normalFromTexture)
{

    vec3 unpackedNormal = normalFromTexture*2.0 -1.0;  // normal in tangent space
    vec3 tangent = vec3(1.0,0.0,0.0);

    vec3 N = normalize(normal);
    vec3 T =  normalize(tangent - N * dot(tangent, N));
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * unpackedNormal);
	//return normalize(mul(unpackedNormal, ));       // normal in world space

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