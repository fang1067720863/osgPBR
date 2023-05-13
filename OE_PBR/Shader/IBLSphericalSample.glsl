

#pragma vp_function SampleSphericalMap, vertex_model
const vec2 invAtan = vec2(0.1591, 0.3183);
out vec2 spherical_uv;
void SampleSphericalMap(inout vec4 vertex) {
    vec3 tmp = normalize(vertex.xyz);

    vec2 uv = vec2(atan(tmp.z, tmp.x), asin(tmp.y));
    uv *= invAtan;
    uv += 0.5;
    spherical_uv = uv;
}

[break]

#pragma vp_function fs, fragment_output
in vec2 spherical_uv;
out vec4 frag;
uniform sampler2D tex;
void fs(inout vec4 c) {
    c = texture(tex, spherical_uv);
    frag = c;
}