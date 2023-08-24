
struct pbr_Material
{
    vec4  baseColorFactor;
    vec3  emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    float alphaMask;
    float alphaMaskCutoff;
    float aoStrength;
#ifdef USE_SHEEN
    vec3 sheenColor;
    float sheenRoughness;
#endif
#ifdef USE_CLEARCOAT
    float clearcoat;
	float clearcoatRoughness;
    vec3 clearcoatF0;
    float clearcoatF90;
#endif
};

struct ReflectedLight
{
	vec3 indirectDiffuse;
	vec3 indirectSpecular;
	vec3 directDiffuse;
	vec3 directSpecular;
#ifdef USE_SHEEN
    vec3 sheenSpecular;
#endif
#ifdef USE_CLEARCOAT
    vec3 clearcoatSpecular;
#endif
};
struct GeometricContext
{
	vec3 normal;
	vec3 viewDir;
#ifdef USE_CLEARCOAT
    vec3 clearcoatNormal;
#endif
};
struct osg_LightSourceParameters 
{   
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   vec4 position;
   vec3 spotDirection;
   float spotExponent;
   float spotCutoff;
   float spotCosCutoff;
   float constantAttenuation;
   float linearAttenuation;
   float quadraticAttenuation;

   bool enabled;
};