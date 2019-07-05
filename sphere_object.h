#ifndef SPHERE_H
#define SPHERE_H

#include "object.h"
#include <glm/glm.hpp>

enum MaterialType {
    Diffuse,
    Metal,
    Dielectric
};

class SphereObject : public object
{
public:
    SphereObject() = default;

    SphereObject(const glm::vec3& center, float radius, MaterialType type, const glm::vec3& albedo, float prop = 0)
        : center(center)
        , radius(radius)
        , type(type)
        , albedo(albedo)
        , prop(prop)
    {
    }

    aabb3 get_aabb() const override
    {
        return { center, radius };
    }

    glm::vec3 center;
    float radius;

    MaterialType type;
    glm::vec3 albedo;
    float prop;
};

#endif // SPHERE_H