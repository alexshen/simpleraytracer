#include "uborenderinput.h"

#include "glslprogram.h"
#include "bvh_node.h"
#include "scene.h"
#include "sphere_object.h"

#include <glm/glm.hpp>
#include <cassert>

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
    float _padding[3];
};

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

} // anonymouse namespace

UboRenderInput::UboRenderInput(const Scene& scene)
{
    assert(scene.objects.size() <= Buffer::MaxObjects);

    Buffer buffer;
    flatten(scene.root.get(), buffer);

    glGenBuffers(1, &m_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);

    constexpr int TotalSize = sizeof(buffer.nodes) + sizeof(buffer.objects) + sizeof(buffer.materials);
    glBufferData(GL_UNIFORM_BUFFER, TotalSize, nullptr, GL_STATIC_DRAW);
    auto p = static_cast<char*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY));

    std::memcpy(p, buffer.nodes, sizeof(buffer.nodes));
    p += sizeof(buffer.nodes);
    std::memcpy(p, buffer.objects, sizeof(buffer.objects));
    p += sizeof(buffer.objects);
    std::memcpy(p, buffer.materials, sizeof(buffer.materials));
    glUnmapBuffer(GL_UNIFORM_BUFFER);
}

UboRenderInput::~UboRenderInput()
{
    glDeleteBuffers(1, &m_ubo);
}

void UboRenderInput::setInput(GLSLProgram& prog) const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_ubo);

    auto index = glGetUniformBlockIndex(prog.getHandle(), "Scene");
    glUniformBlockBinding(prog.getHandle(), index, 0);
}