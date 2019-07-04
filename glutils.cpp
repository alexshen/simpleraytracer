#include "glutils.h"

#include "glslprogram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdio>
#include <string>

using std::string;

namespace GLUtils {

void APIENTRY debugCallback( GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar * msg, const void * param ) {
	
	string sourceStr;
	switch(source) {
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		sourceStr = "WindowSys";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		sourceStr = "App";
		break;
	case GL_DEBUG_SOURCE_API:
		sourceStr = "OpenGL";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		sourceStr = "ShaderCompiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		sourceStr = "3rdParty";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		sourceStr = "Other";
		break;
	default:
		sourceStr = "Unknown";
	}
	
	string typeStr;
	switch(type) {
	case GL_DEBUG_TYPE_ERROR:
		typeStr = "Error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		typeStr = "Deprecated";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		typeStr = "Undefined";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		typeStr = "Portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		typeStr = "Performance";
		break;
	case GL_DEBUG_TYPE_MARKER:
		typeStr = "Marker";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		typeStr = "PushGrp";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		typeStr = "PopGrp";
		break;
	case GL_DEBUG_TYPE_OTHER:
		typeStr = "Other";
		break;
	default:
		typeStr = "Unknown";
	}
	
	string sevStr;
	switch(severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		sevStr = "HIGH";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		sevStr = "MED";
		break;
	case GL_DEBUG_SEVERITY_LOW:
		sevStr = "LOW";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		sevStr = "NOTIFY";
		break;
	default:
		sevStr = "UNK";
	}
	
	printf("%s:%s[%s](%d): %s\n", sourceStr.c_str(), typeStr.c_str(), sevStr.c_str(), 
		id, msg);		
#ifdef _MSC_VER
    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        __debugbreak();
    }
#endif
}

void loadProgram(const char * path, GLSLProgram & program)
{
    using namespace std::string_literals;
    program.compileShader((path + ".vs"s).c_str());
    program.compileShader((path + ".fs"s).c_str());
    program.link();
}

glm::mat4 rotate(glm::mat4 m, const glm::vec3& euler)
{
    m = glm::rotate(m, glm::radians(euler.y), glm::vec3(0,1,0));
    m = glm::rotate(m, glm::radians(euler.x), glm::vec3(1,0,0));
    return glm::rotate(m, glm::radians(euler.z), glm::vec3(0,0,1));
}

int checkForOpenGLError(const char * file, int line) {
    //
    // Returns 1 if an OpenGL error occurred, 0 otherwise.
    //
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
		const char * message = "";
		switch( glErr )
		{
		case GL_INVALID_ENUM:
			message = "Invalid enum";
			break;
		case GL_INVALID_VALUE:
			message = "Invalid value";
			break;
		case GL_INVALID_OPERATION:
			message = "Invalid operation";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			message = "Invalid framebuffer operation";
			break;
		case GL_OUT_OF_MEMORY:
			message = "Out of memory";
			break;
		default:
			message = "Unknown error";
		}

        printf("glError in file %s @ line %d: %s\n", file, line, message);
        retCode = 1;
        glErr = glGetError();
    }
    return retCode;
}

void dumpGLInfo(bool dumpExtensions) {
    const GLubyte *renderer = glGetString( GL_RENDERER );
    const GLubyte *vendor = glGetString( GL_VENDOR );
    const GLubyte *version = glGetString( GL_VERSION );
    const GLubyte *glslVersion = glGetString( GL_SHADING_LANGUAGE_VERSION );

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
	
	printf("-------------------------------------------------------------\n");
    printf("GL Vendor    : %s\n", vendor);
    printf("GL Renderer  : %s\n", renderer);
    printf("GL Version   : %s\n", version);
    printf("GL Version   : %d.%d\n", major, minor);
    printf("GLSL Version : %s\n", glslVersion);
    printf("-------------------------------------------------------------\n");

    if( dumpExtensions ) {
        GLint nExtensions;
        glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);
        for( int i = 0; i < nExtensions; i++ ) {
            printf("%s\n", glGetStringi(GL_EXTENSIONS, i));
        }
    }
}

GLuint createFullScreenQuad()
{
    // Array for quad
    GLfloat verts[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
    };
    GLfloat tc[] = {
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };

    // Set up the buffers
    unsigned int handle[2];
    glGenBuffers(2, handle);

    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);

	GLuint quad;
    // Set up the vertex array object
    glGenVertexArrays( 1, &quad );
    glBindVertexArray(quad);

    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glVertexAttribPointer( (GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray(0);  // Vertex position

    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glVertexAttribPointer( (GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray(2);  // Texture coordinates

    glBindVertexArray(0);

	return quad;
}

void renderFullScreenQuad(GLuint quad)
{
	glBindVertexArray(quad);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void checkFramebufferStatus(const char* file, int lineno)
{
    if (int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        status != GL_FRAMEBUFFER_COMPLETE) {
#define CASE_STATUS(status) case status: printf("file: %s, line: %d, " #status "\n", file, lineno); break;
        switch (status)
        {
            CASE_STATUS(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
            CASE_STATUS(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
            CASE_STATUS(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)
            CASE_STATUS(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER)
            CASE_STATUS(GL_FRAMEBUFFER_UNSUPPORTED)
            CASE_STATUS(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE)
            CASE_STATUS(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS)
        }
		std::exit(1);
    }
#undef CASE_STATUS
}

} // namespace GLUtils
