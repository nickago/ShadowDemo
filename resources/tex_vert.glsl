#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
out vec3 fragNorVert;
out vec3 position;
out vec2 TexCoords;

void main()
{
	gl_Position = P * V * M * vertPos;
    fragNorVert = (V * M * vec4(vertNor, 0.0)).xyz;
    position = (V * M * vertPos).xyz;
    TexCoords = vertPos.xy;
}
