#version 410

layout(location = 0) in vec4 Position;

uniform mat4 MVP;

void main()
{
    gl_Position = MVP * Position;
}