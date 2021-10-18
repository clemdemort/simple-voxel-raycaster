#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;
out vec2 FragCoord;
//this will create a triangle with a big enough size that it covers the entire screen
void main()
{
    gl_Position = vec4(aPos, 1.0);
    FragCoord = aPos.xy;
    ourColor = aColor;
}