#version 410

layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;

uniform mat4 MVP;

out vec2 vST;

void main()
{
    vST = TexCoord;
    gl_Position = MVP * Position;
}