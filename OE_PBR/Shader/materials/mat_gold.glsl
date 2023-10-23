
uniform sampler2D T_Metal_Gold_N;
uniform sampler2D T_Metal_Gold_D;
uniform sampler2D T_MacroVariation;

vec2 uv_1 = oe_texcoord * vec2(0.2134);
vec2 uv_2 = oe_texcoord * vec2(0.05341);
vec2 uv_3 = oe_texcoord * vec2(0.002);

float color1 = texture(T_MacroVariation, uv_1).r + 0.5;
float color2 = texture(T_MacroVariation, uv_2).r + 0.5;
float color3 = texture(T_MacroVariation, uv_3).r + 0.5;

float color_r = color1 * color2 * color3;

vec3 color_m = mix(vec3(0.5),vec3(1.0),color_r);

vec4 color_d = texture(T_Metal_Gold_D, oe_texcoord).rgba;
color_d = SRGBtoLINEAR(color_d);

diffuseColor = color_d.rgb * color_m;
metallic *= 1.0;
roughness *= mix(0.3,0.5, color_d.a*color_r);

