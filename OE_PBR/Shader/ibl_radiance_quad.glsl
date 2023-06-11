

#pragma vp_function vs, vertex_model
out vec3 WorldPos;
void vs(inout vec4 vertex) {
    WorldPos = normalize(vertex.xyz);
}
[break]



#pragma vp_function fs, fragment_output

out vec4 FragColor;
in vec3 WorldPos;
#define PI 3.14159265359f
uniform sampler2D environmentMap;
// uniform float roughness;
// uniform int ngt;
#pragma include IBLFunc.glsl

const vec2 invAtan = vec2(0.1591, 0.3183);

 const uint ConvolutionSampleCount = 1024u;
 const uint ConvolutionMaxSamples = 1024u;

vec2 sphericalUV(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan; uv += vec2(0.5); return uv;
}

const vec2 Atan = vec2(6.28318, 3.14159);
vec3 invSphericalUV(vec2 v)
{
    vec2 uv = (v - vec2(0.5)) * Atan; float cosT = cos(uv.y);
    return vec3(cosT * cos(uv.x), sin(uv.y), cosT * sin(uv.x));
}




void
sumSpecular (vec3 hdrPixel, float NoL, inout vec4 result)
{
    result.xyz += (hdrPixel * NoL);
    result.w += NoL;
}


vec3 ImportanceSample (vec3 R )
{
    vec3 N = R;
    vec3 V = R;
    vec4 result = vec4(0,0,0,0);

   
    float ConvolutionRoughness = 5.5;
    float ConvolutionSamplesOffset = 0.0;

    uint SampleStep = uint(ConvolutionMaxSamples / ConvolutionSampleCount);
    uint sampleId = uint(ConvolutionSamplesOffset);

   const uint cubeWidth = 512u;
  

    for(uint i = 0; i < ConvolutionSampleCount; i++ )
    {
        vec2 Xi = Hammersley(sampleId, ConvolutionMaxSamples);

        vec3 H = ImportanceSampleGGX( Xi, N, ConvolutionRoughness);
        vec3 L = 2 * dot( V, H ) * H - V;
        float NoL = max(dot( N, L ), 0.0);
        float VoL = max (dot(V, L), 0.0);
        float NoH = max(dot( N, H ), 0.0);
        float VoH = max(dot( V, H ), 0.0);
        if( NoL > 0.0 )
        {
            //
            // Compute pdf of BRDF
            // Taken from Epic's Siggraph 2013 Lecture:
            // http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
            //
            float Dh = specularD(ConvolutionRoughness, NoH);
            float pdf = Dh * NoH / (4*VoH);
            float solidAngleTexel = 4 * PI / (6 * cubeWidth * cubeWidth);
            float solidAngleSample = 1.0 / (ConvolutionSampleCount * pdf);
            float lod = ConvolutionRoughness == 0.0 ? 0.0 : 0.5 * log2(float(solidAngleSample/solidAngleTexel));

            //vec3 hdrPixel = rescaleHDR(ConvolutionSrc.SampleLevel(EnvMapSampler, L, lod).rgb);
            vec3 hdrPixel = textureLod(environmentMap, sphericalUV(L), 4.0).rgb;

            sumSpecular(hdrPixel, 0.5, result);
        }
        sampleId += SampleStep;
   }

    if (result.w == 0.0)
        return result.xyz;
   else
       return (result.xyz / result.w);
}

void fs(inout vec4 c) {

    vec3 N = normalize(WorldPos.xyz);
    vec3 prefilteredColor = ImportanceSample(N);

    FragColor = vec4(prefilteredColor, 1.0);
}

// void fs(inout vec4 c) {
//     vec3 N = normalize(WorldPos.xyz);
//     float roughness = 0.0f;
    
    
//     // make the simplifying assumption that V equals R equals the normal 
//     vec3 R = N;
//     vec3 V = R;

//     const uint SAMPLE_COUNT = 1024u;
//     vec3 prefilteredColor = vec3(0.0);
//     float totalWeight = 0.0;
    
//     for(uint i = 0u; i < SAMPLE_COUNT; ++i)
//     {
//         // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
//         vec2 Xi = Hammersley(i, SAMPLE_COUNT);
//         vec3 H = ImportanceSampleGGX(Xi, N, roughness);
//         vec3 L  = normalize(2.0 * dot(V, H) * H - V);

//         float NdotL = max(dot(N, L), 0.0);
//         if(NdotL > 0.0)
//         {
//             // sample from the environment's mip level based on roughness/pdf
//             float D   = DistributionGGX(N, H, roughness);
//             float NdotH = max(dot(N, H), 0.0);
//             float HdotV = max(dot(H, V), 0.0);
//             float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

//             float resolution = 512.0; // resolution of source cubemap (per face)
//             float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
//             float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

//             float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
            
//             prefilteredColor += textureLod(environmentMap, sphericalUV(L), 0.5).rgb * NdotL;
//             totalWeight      += NdotL;
//         }
//     }

//     prefilteredColor = prefilteredColor / totalWeight;

//     FragColor = vec4(prefilteredColor, 1.0);
// }





