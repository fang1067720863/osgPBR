#pragma vp_function vs, vertex_model
out vec3 WorldPos;
out vec2 TexCoords;
void vs(inout vec4 vertex) {
    WorldPos = normalize(vertex.xyz);
    TexCoords =  gl_MultiTexCoord0.xy;
}
[break]

#pragma vp_function fs, fragment_output
out vec4 frag;
in vec3 WorldPos;
in vec2 TexCoords;
uniform samplerCube IrradianceMap;

uniform float mipLevel;
uniform int colorMap;

const vec2 Atan = vec2(6.28318, 3.14159);
vec3 invSphericalUV(vec2 v)
{
    vec2 uv = (v - vec2(0.5)) * Atan; float cosT = cos(uv.y);
    return vec3(cosT * cos(uv.x), sin(uv.y), cosT * sin(uv.x));
}

void fs(inout vec4 c) {
    
    vec3 N = normalize(invSphericalUV(TexCoords.xy));
    // vec3 N = invSphericalUV(TexCoords.xy);
    // N = normalize(vec3(-N.x,N.y,N.z));
    // vec3 up = vec3(0.0, 1.0, 0.0);
    // vec3 right = cross(up, N); up = cross(N, right);  // tangent space calculation

    // vec3 sampleVec = tangentVec.x * right + tangentVec.y * up + tangentVec.z * N;
    vec3 irradiance = textureLod(IrradianceMap, N, mipLevel).rgb;
    c = vec4(irradiance, 1.0f);
    frag = c;
}
