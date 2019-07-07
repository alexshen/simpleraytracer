#version 410

in vec2 vST;

uniform sampler2D MainTex;

out vec4 FragColor;

void main()
{
    FragColor = texture(MainTex, vST);
}