

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