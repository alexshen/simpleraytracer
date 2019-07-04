#version 410

// assume the camera is positioned at origin
// focal length

uniform vec3 CameraPos;
uniform vec3 CameraLookAt;
uniform float FocalLength; // focal length in world unit
uniform float FovY; // fov in rad
uniform vec2 ScreenSize;
uniform vec3 BackgroundColor;
// the current rendering window
uniform vec4 Window;
uniform int NumSamples;
uniform int NumSpheres;

uniform int IterStart;
uniform int IterNum;

// Material types
const int TypeDiffuse = 0;
const int TypeMetal = 1;
const int TypeDielectric = 2;

struct Material {
    // xyz = albeo color
    // w = material type
    vec4 albedo;
    // other properties
    // for metal, fuzziness
    // for dielectric, refraction index
    float prop;
};

const int MaxObjects = 512;

layout(std140) uniform Scene {
    vec4 spheres[MaxObjects];
    // xyz, material color
    // w, material type
    Material materials[MaxObjects];
};

layout(location = 0) out vec4 FragColor;

vec3 g_CameraLookDir;
vec3 g_CameraUp;
vec3 g_CameraRight;
vec2 g_halfImageSize;

// return a random number in the range [0, 1)
int g_seed = 0;
float random()
{
    // https://software.intel.com/en-us/articles/fast-random-number-generator-on-the-intel-pentiumr-4-processor
    g_seed = 214013*g_seed+2531011;
    return ((g_seed>>16)&0x7FFF) / float(0x8000);
}

// return a random point in the unit rect [0,1)x[0,1)
vec2 randomInUnitRect()
{
    return vec2(random(), random());
}

vec3 randomInUnitSphere()
{
    vec3 res;
    do {
        res = vec3(random(), random(), random());
        res = res * 2 - 1;
    } while (dot(res, res) >= 1.0);
    return res;
}

struct Ray {
    vec3 origin;
    vec3 dir;
};

// find the first intersection between the ray and the sphere
// if no insertection is found, return -1
float intersectSphere(vec4 sphere, Ray ray, float tmin, float tmax)
{
    vec3 oc = ray.origin - sphere.xyz;
    float a = dot(ray.dir, ray.dir);
    float b = dot(oc, ray.dir);
    float c = dot(oc, oc) - sphere.w * sphere.w;
    float discriminant = b*b - a*c;
    if (discriminant < 0) {
        return -1;
    }
    float sqrtDisr = sqrt(discriminant);
    float t = (-b - sqrtDisr) / a;
    if (tmin <= t && t <= tmax) {
        return t;
    }
    t = (-b + sqrtDisr) / a;
    if (tmin <= t && t <= tmax) {
        return t;
    }
    return -1;
}

vec3 getRayDir(vec2 jitter)
{
    vec2 p = (jitter + gl_FragCoord.xy) / ScreenSize;
    p = p * 2 - 1;
    vec3 dir = g_CameraLookDir * FocalLength + 
                p.x * g_CameraRight * g_halfImageSize.x + 
                p.y * g_CameraUp * g_halfImageSize.y;
    return normalize(dir);
}

vec3 getBackgroundColor(vec3 dir)
{
    float t = (dir.y + 1.0) * 0.5;
    return mix(vec3(1), BackgroundColor, t);
}

struct HitRecord {
    vec3 pt;
    vec3 normal;
    Material material;
};

const float Infinity = intBitsToFloat(0x7f800000);
bool hit(Ray ray, float tmin, float tmax, out HitRecord rec)
{
    int sphereIndex = -1;
    float minT = Infinity;
    for (int i = 0; i < NumSpheres; ++i) {
        float t = intersectSphere(spheres[i], ray, tmin, tmax);
        if (t != -1 && minT > t) {
            minT = t;
            sphereIndex = i;
        }
    }
    if (sphereIndex != -1) {
        rec.pt = ray.origin + minT * ray.dir;
        rec.normal = normalize(rec.pt - spheres[sphereIndex].xyz);
        rec.material = materials[sphereIndex];
        return true;
    }
    return false;
}

bool diffuseScatter(vec3 rayDir, HitRecord rec, out vec3 attenuation, out vec3 scattered)
{
    scattered = normalize(rec.normal + randomInUnitSphere()); 
    attenuation = rec.material.albedo.rgb;
    return true;
}

float schlick(float cosine, float n)
{
    float r0 = (1 - n) / (1 + n);
    r0 *= r0;
    return r0 + (1.0f - r0) * pow(1.0f - cosine, 5);
}

bool metalScatter(vec3 rayDir, HitRecord rec, out vec3 attenuation, out vec3 scattered)
{
    scattered = rec.material.prop * randomInUnitSphere() + reflect(rayDir, rec.normal);
    scattered = normalize(scattered);
    attenuation = rec.material.albedo.rgb;
    return dot(rec.normal, scattered) > 0;
}

bool dielectricScatter(vec3 rayDir, HitRecord rec, out vec3 attenuation, out vec3 scattered)
{
    vec3 uin = normalize(rayDir);
    attenuation = vec3(1);

    float index = rec.material.prop;
    float ni_over_nt;
    float cosine = dot(uin, rec.normal);
    vec3 normal;
    if (cosine > 0) {
        ni_over_nt = index;
        normal = -rec.normal;
    } else {
        ni_over_nt = 1.0f / index;
        normal = rec.normal;
    }

    float reflect_prob;
    vec3 refracted = refract(uin, normal, ni_over_nt);
    // if > 0, then we assume that light travels from denser material to air
    if (cosine > 0) {
        cosine = sqrt(1.0f - index * index * (1.0f - cosine * cosine));
    } else {
        cosine = -cosine;
    }
    reflect_prob = schlick(cosine, index);

    if (random() < reflect_prob) {
        scattered = reflect(uin, normal);
    } else {
        scattered = refracted;
    }
    return true;
}

vec3 raytrace(vec2 jitter)
{
    const int MaxIter = 50;

    vec3 color = vec3(1);
    HitRecord rec;
    Ray ray = Ray(CameraPos, getRayDir(jitter));
    int i;
    for (i = 0; i < MaxIter; ++i) {
        if (hit(ray, 0.0001, Infinity, rec)) {
            vec3 scatteredDir;
            vec3 attenuation;
            bool scattered = false;
            switch (int(rec.material.albedo.w)) {
            case TypeDiffuse:
                scattered = diffuseScatter(ray.dir, rec, attenuation, scatteredDir);
                break;

            case TypeMetal:
                scattered = metalScatter(ray.dir, rec, attenuation, scatteredDir);
                break;

            case TypeDielectric:
                scattered = dielectricScatter(ray.dir, rec, attenuation, scatteredDir);
                break;
            }
            if (scattered) {
                ray.origin = rec.pt;
                ray.dir = scatteredDir;
                color *= attenuation;
            } else {
                color = vec3(0);
                break;
            }
        } else {
            break;
        }
    }

    color *= getBackgroundColor(ray.dir);
    return color;
}

void main()
{
    g_CameraLookDir = normalize(CameraLookAt - CameraPos);
    g_CameraRight = cross(g_CameraLookDir, vec3(0, 1, 0));
    g_CameraUp = cross(g_CameraRight, g_CameraLookDir);

    g_halfImageSize.y = tan(FovY / 2) * FocalLength;
    g_halfImageSize.x = g_halfImageSize.y * ScreenSize.x / ScreenSize.y;

    vec3 color = vec3(0);
#ifdef BLOCK_REFINE
    g_seed = 0;
    if (all(greaterThanEqual(gl_FragCoord.xy, Window.xy)) && 
        all(lessThan(gl_FragCoord.xy, Window.zw))) {
        for (int i = 0; i < NumSamples; ++i) {
            color += raytrace(randomInUnitRect());
        }

        FragColor = vec4(color / NumSamples, 1);
    } else {
        discard;
    }
#else
    g_seed = IterStart * 17 + IterNum;;
    int iterEnd = min(NumSamples, IterStart + IterNum);
    for (int i = IterStart; i < iterEnd; ++i) {
        color += raytrace(randomInUnitRect());
    }

    FragColor = vec4(color / NumSamples, 1);
#endif
}