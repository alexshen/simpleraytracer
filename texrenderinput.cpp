#include "texrenderinput.h"
#include "scene.h"
#include "glslprogram.h"
#include "bvh_node.h"
#include <glm/glm.hpp>
#include <iterator>
#include <cassert>

namespace
{

struct NodeData
{
    int left;
    int right;
    int firstObjectIndex;
    int numObjects;
};

struct AABB {
    glm::vec3 min; float _pad0;
    glm::vec3 max; float _pad1;
};

struct Material {
    glm::vec3 albedo;
    float type;
    float prop;
    float _pad[3];
};

struct State
{
    std::vector<AABB> aabbs;
    std::vector<NodeData> nodeData;
    std::vector<glm::vec4> objects;
    std::vector<Material> materials;
};

int flatten(const bvh_node* root, State& state)
{
    if (!root) {
        return -1;
    }

    auto& aabb = state.aabbs.emplace_back();
    aabb.min = root->get_aabb().min;
    aabb.max = root->get_aabb().max;

    int index = (int)state.nodeData.size();
    state.nodeData.emplace_back();
    NodeData nodeData;
    nodeData.numObjects = root->num_objects();
    nodeData.firstObjectIndex = (int)state.objects.size();

    for (int i = 0; i < root->num_objects(); ++i) {
        auto obj = static_cast<const SphereObject*>(root->get_object(i));
        state.objects.emplace_back(obj->center, obj->radius);
        state.materials.push_back({
            obj->albedo,
            (float)obj->type,
            obj->prop
        });
    }

    nodeData.left = flatten(root->left(), state);
    nodeData.right = flatten(root->right(), state);
    state.nodeData[index] = nodeData;

    return index;
}

}

TextureRenderInput::TextureRenderInput(const Scene& scene)
{
    State state;
    flatten(scene.root.get(), state);

    GLint maxSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    assert((int)state.aabbs.size() * 2 <= maxSize);
    assert((int)state.materials.size() * 2 <= maxSize);

    glGenTextures(1, &m_nodeAABBTex);
    glBindTexture(GL_TEXTURE_1D, m_nodeAABBTex);
    // aabb min, max, vec4
    glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGBA32F, state.aabbs.size() * 2);
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, state.aabbs.size() * 2, GL_RGBA, GL_FLOAT, state.aabbs.data());
    
    glGenTextures(1, &m_nodeDataTex);
    glBindTexture(GL_TEXTURE_1D, m_nodeDataTex);
    // left, right, first, num
    glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGBA32I, state.nodeData.size());
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, state.nodeData.size(), GL_RGBA_INTEGER, GL_INT, state.nodeData.data());

    glGenTextures(1, &m_objectTex);
    glBindTexture(GL_TEXTURE_1D, m_objectTex);
    // center and radius
    glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGBA32F, state.objects.size());
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, state.objects.size(), GL_RGBA, GL_FLOAT, state.objects.data());

    glGenTextures(1, &m_materialTex);
    glBindTexture(GL_TEXTURE_1D, m_materialTex);
    glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGBA32F, state.materials.size() * 2);
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, state.materials.size() * 2, GL_RGBA, GL_FLOAT, state.materials.data());
}

TextureRenderInput::~TextureRenderInput()
{
    GLuint texs[4] = {
        m_nodeAABBTex,
        m_nodeDataTex,
        m_objectTex,
        m_materialTex
    };
    glDeleteTextures(std::size(texs), texs);
}

void TextureRenderInput::setInput(GLSLProgram& prog) const
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, m_nodeAABBTex);
    prog.setUniform("NodeAABBTex", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, m_nodeDataTex);
    prog.setUniform("NodeDataTex", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, m_objectTex);
    prog.setUniform("ObjectTex", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_1D, m_materialTex);
    prog.setUniform("MaterialTex", 3);
}