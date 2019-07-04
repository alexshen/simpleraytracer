#pragma once

#include <glm/fwd.hpp>
#include <glad/glad.h>

class GLSLProgram;

namespace GLUtils
{
    int checkForOpenGLError(const char *, int);
    
    void dumpGLInfo(bool dumpExtensions = false);
    
    void APIENTRY debugCallback( GLenum source, GLenum type, GLuint id,
		GLenum severity, GLsizei length, const GLchar * msg, const void * param );

    void loadProgram(const char* path, GLSLProgram& program);

    // yaw-y, pitch-x, roll-z
    glm::mat4 rotate(glm::mat4 m, const glm::vec3& euler);

    GLuint createFullScreenQuad();
    void renderFullScreenQuad(GLuint quad);
    // check the completeness of the currently bound draw framebuffer
    void checkFramebufferStatus(const char* file, int lineno);
}

#define GL_CHECK_ERROR GLUtils::checkForOpenGLError(__FILE__, __LINE__)
#define GL_CHECK_FRAMEBUFFER_STATUS GLUtils::checkFramebufferStatus(__FILE__, __LINE__)