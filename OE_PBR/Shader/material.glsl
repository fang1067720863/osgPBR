#pragma vp_function pbr_material_input, fragment, first

struct pbr_Material
{
    vec4  baseColorFactor;
    vec3  emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    float alphaMask;
    float alphaMaskCutoff;
    float aoStrength;
}oe_pbr;
void pbr_material_input(inout vec4 ignore_me)
{
    oe_pbr.baseColorFactor = vec4(1.0,1.0,1.0,1.0);
    oe_pbr.emissiveFactor = vec3(0.0,0.0,1.0);
    oe_pbr.metallicFactor = 0.0;
    oe_pbr.roughnessFactor = 0.5;
    oe_pbr.alphaMask = 0.1;
    oe_pbr.alphaMaskCutoff = 0.1;
    oe_pbr.aoStrength = 0.1;
}