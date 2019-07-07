#ifndef SCENE_H
#define SCENE_H

#pragma once

#include "bvh_node.h"
#include "sphere_object.h"
#include <vector>

struct Scene
{
    std::vector<SphereObject> objects;
    std::unique_ptr<bvh_node> root;
};

struct Node
{
    glm::vec3 min; float _pad0;
    glm::vec3 max; float _pad1;

    int left; // left node index
    int right; // right node index
    int firstObjIndex;
    int numObj;
};

struct Sphere
{
    glm::vec3 center;
    float radius;
};

struct Material
{
    glm::vec3 albedo;
    float type;

    float prop;
    float _padding[3];
};

struct SceneBuffer
{
    SceneBuffer(const Scene& scene);

    std::vector<Node> nodes;
    std::vector<Sphere> objects;
    std::vector<Material> materials;
};

Scene createScene();

#endif // SCENE_H
