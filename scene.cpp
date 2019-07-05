#include "scene.h"
#include "utils.h"
#include "bvh_node.h"
#include "sphere_object.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <cstring>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <chrono>

namespace
{

struct Node {
    glm::vec3 min; float _pad0;
    glm::vec3 max; float _pad1;

    int left; // left node index
    int right; // right node index
    int firstObjIndex;
    int numObj;
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

struct Scene
{
    std::vector<SphereObject> objects;
    std::unique_ptr<bvh_node> root;
};

Scene createScene()
{
    Scene scene;
    scene.objects.push_back({ glm::vec3(0, -1000, 0), 1000, Diffuse, glm::vec3(1) * 0.5f });

    const int x_count = 11, y_count = 11;
    for (int a = -x_count; a < x_count; ++a) {
        for (int b = -y_count; b < y_count; ++b) {
            float choose_mat = utils::random();
            glm::vec3 center(a + 0.9f * utils::random(), 0.2f, b + 0.9f * utils::random());
            if ((center - glm::vec3(4, 0.2f, 0)).length() > 0.9f) {
                SphereObject obj;
                obj.center = center;
                obj.radius = 0.2f;
                if (choose_mat < 0.8f) {
                    obj.albedo = glm::vec3(utils::random() * utils::random(),
                                           utils::random() * utils::random(),
                                           utils::random() * utils::random());
                    obj.type = Diffuse;
                } else if (choose_mat < 0.95f) {
                    obj.albedo = glm::vec3(0.5f * (1 + utils::random()),
                                           0.5f * (1 + utils::random()),
                                           0.5f * (1 + utils::random()));
                    obj.prop = 0.5f * utils::random();
                    obj.type = Metal;
                } else {
                    obj.type = Dielectric;
                    obj.prop = 1.5f;
                }
                scene.objects.push_back(obj);
            }
        }
    }

    scene.objects.push_back({ glm::vec3(0, 1, 0), 1.0f, Dielectric, glm::vec3(0), 1.5f });
    scene.objects.push_back({ glm::vec3(-4, 1, 0), 1.0f, Diffuse, glm::vec3(0.4, 0.2, 0.1) });
    scene.objects.push_back({ glm::vec3(4, 1, 0), 1.0f, Metal, glm::vec3(0.7, 0.6, 0.5) });

    std::vector<object*> objects;
    for (auto& o : scene.objects) {
        objects.push_back(&o);
    }
    scene.root = std::make_unique<bvh_node>(objects.data(), objects.size());
    return scene;
}

struct Buffer {
    static constexpr int MaxObjects = 512;
    static constexpr int MaxNodes = 640;
    
    Node nodes[MaxNodes];
    Sphere objects[MaxObjects];
    Material materials[MaxObjects];

    int numNodes = 0;
    int numObjects = 0;
};

int flatten(const bvh_node* root, Buffer& buffer)
{
    if (!root) {
        return -1;
    }

    assert(buffer.numNodes < Buffer::MaxNodes);
    assert(buffer.numObjects + root->num_objects() <= Buffer::MaxObjects);

    int index = buffer.numNodes++;
    auto& cur = buffer.nodes[index];
    cur.min = root->get_aabb().min;
    cur.max = root->get_aabb().max;
    cur.firstObjIndex = buffer.numObjects;
    cur.numObj = root->num_objects();

    for (int i = 0; i < root->num_objects(); ++i, ++buffer.numObjects) {
        auto obj = static_cast<const SphereObject*>(root->get_object(i));
        auto& target = buffer.objects[buffer.numObjects];
        target.center = obj->center;
        target.radius = obj->radius;

        auto& mat = buffer.materials[buffer.numObjects];
        mat.albedo = obj->albedo;
        mat.type = static_cast<float>(obj->type);
        mat.prop = obj->prop;
    }

    cur.left = flatten(root->left(), buffer);
    cur.right = flatten(root->right(), buffer);

    return index;
}

}

SceneUbo createSceneUniformBuffer()
{
    auto scene = createScene();
    assert(scene.objects.size() <= Buffer::MaxObjects);

    Buffer buffer;
    flatten(scene.root.get(), buffer);

    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);

    constexpr int TotalSize = sizeof(buffer.nodes) + sizeof(buffer.objects) + sizeof(buffer.materials);
    glBufferData(GL_UNIFORM_BUFFER, TotalSize, nullptr, GL_STATIC_DRAW);
    auto p = static_cast<char*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY));

    std::memcpy(p, buffer.nodes, sizeof(buffer.nodes));
    p += sizeof(buffer.nodes);
    std::memcpy(p, buffer.objects, sizeof(buffer.objects));
    p += sizeof(buffer.objects);
    std::memcpy(p, buffer.materials, sizeof(buffer.materials));
    glUnmapBuffer(GL_UNIFORM_BUFFER);

    return { ubo, (int)scene.objects.size() };
}