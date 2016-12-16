#version 330 core 
in vec3 fragNorVert;
in vec3 position;
in vec2 TexCoords;
uniform vec3 light;
uniform mat4 V;
uniform vec4 Lc;
out vec4 color;

uniform sampler2D tex;

void main()
{
    vec3 view_light = (V * vec4(light, 1.0f)).xyz;
    vec3 light_vec = normalize(view_light - position);
    vec3 fragNor = normalize(fragNorVert);
    vec3 view_dir = normalize(-position);
    vec3 R = reflect(-light_vec, fragNor);

    float specular = max(0, dot(view_dir, R));
    float difuse = max(0, dot(light_vec, fragNor));

    color = 0.5 * (specular + difuse) * texture(tex, TexCoords) * Lc;
}
