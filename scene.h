#ifndef SCENE_H
#define SCENE_H

#pragma once

#include <glad/glad.h>

struct SceneUbo
{
    GLuint handle;
    int numSpheres;
};

SceneUbo createSceneUniformBuffer();

#endif // SCENE_H
