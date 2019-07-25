#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 transform;
uniform float xOffset;
uniform float yOffset;

void main()
{
	gl_Position = transform * vec4(aPos.x + xOffset, aPos.y + yOffset, aPos.z, 1.0);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
