#include "scene.h"
#include "utils.h"

#include <glm/glm.hpp>
#include <cstring>

namespace
{

enum Type : int {
    Diffuse,
    Metal,
    Dielectric
};

struct Sphere {
    glm::vec3 center;
    float radius;
};

struct Material {
    glm::vec3 albedo;
    float type;

    float prop;
    float padding[3];
};

}

SceneUbo createSceneUniformBuffer()
{
    constexpr int MaxObjects = 512;
    Sphere spheres[MaxObjects];
    Material materials[MaxObjects];

    spheres[0] = { glm::vec3(0, -1000, 0), 1000, };
    materials[0] = { glm::vec3(1) * 0.5f, Diffuse };
    int i = 1;

    const int x_count = 11, y_count = 11;
    for (int a = -x_count; a < x_count; ++a) {
        for (int b = -y_count; b < y_count; ++b) {
            float choose_mat = utils::random();
            glm::vec3 center(a + 0.9f * utils::random(), 0.2f, b + 0.9f * utils::random());
            if ((center - glm::vec3(4, 0.2f, 0)).length() > 0.9f) {
                spheres[i] = { center, 0.2f };
                auto& mat = materials[i];
                ++i;
                if (choose_mat < 0.8f) {
                    mat.albedo = glm::vec3(utils::random() * utils::random(),
                                           utils::random() * utils::random(),
                                           utils::random() * utils::random());
                    mat.type = Diffuse;
                } else if (choose_mat < 0.95f) {
                    mat.albedo = glm::vec3(0.5f * (1 + utils::random()),
                                           0.5f * (1 + utils::random()),
                                           0.5f * (1 + utils::random()));
                    mat.prop = 0.5f * utils::random();
                    mat.type = Metal;
                } else {
                    mat.type = Dielectric;
                    mat.prop = 1.5f;
                }
            }
        }
    }

    spheres[i] = { glm::vec3(0, 1, 0), 1.0f };
    materials[i] = { glm::vec3(0), Dielectric, 1.5f };
    ++i;

    spheres[i] = { glm::vec3(-4, 1, 0), 1.0f };
    materials[i] = { glm::vec3(0.4, 0.2, 0.1), Diffuse };
    ++i;

    spheres[i] = { glm::vec3(4, 1, 0), 1.0f };
    materials[i] = { glm::vec3(0.7, 0.6, 0.5), Metal };
    ++i;

    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    glBufferData(GL_UNIFORM_BUFFER, (sizeof(Sphere) + sizeof(Material)) * MaxObjects, nullptr, GL_STATIC_DRAW);
    auto p = static_cast<char*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY));
    std::memcpy(p, spheres, sizeof(spheres));
    std::memcpy(p + sizeof(Sphere) * MaxObjects, materials, sizeof(materials));
    glUnmapBuffer(GL_UNIFORM_BUFFER);

    return { ubo, i };
}