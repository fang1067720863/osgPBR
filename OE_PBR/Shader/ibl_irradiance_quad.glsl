#pragma vp_function vs, vertex_model
out vec3 WorldPos;
out vec2 TexCoords;
out vec3 Normal;
void vs(inout vec4 vertex) {
    WorldPos = normalize(vertex.xyz);
    TexCoords =  gl_MultiTexCoord0.xy;
    Normal = normalize(gl_Normal);
}
[break]

#pragma vp_function fs, fragment_output
in vec3 WorldPos;
in vec2 TexCoords;
in vec3 Normal;
out vec4 FragColor;
#define PI 3.14159265359f
uniform sampler2D environmentMap;


const vec2 invAtan = vec2(0.1591, 0.3183);
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

vec3 ImportanceSampleGGX
void fs(inout vec4 c) {
    vec3 N = normalize(WorldPos.xyz);

    vec3 irradiance = vec3(0.0);   
    
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));
       
    float sampleDelta = 0.025;
    float nrSamples = 0.0f;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        // for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        // {
            float theta = 0.25*PI;
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(environmentMap, sphericalUV(sampleVec)).rgb * cos(theta) * sin(theta);
            nrSamples++;
        //}
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    // float phi = PI;
    // float theta = 0.25 * PI;
    // vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
    // // tangent space to world
    // vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

    // irradiance += texture(environmentMap, sphericalUV(sampleVec)).rgb * cos(theta) * sin(theta);

    
    FragColor = vec4(irradiance, 1.0);
}