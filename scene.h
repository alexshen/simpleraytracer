#ifndef SCENE_H
#define SCENE_H

#pragma once

#include "bvh_node.h"
#include "sphere_object.h"

struct Scene
{
    std::vector<SphereObject> objects;
    std::unique_ptr<bvh_node> root;
};

Scene createScene();

#endif // SCENE_H
