#version 330 core 
in vec3 fragNorVert;
in vec3 position;
in vec4 ShadowCoord;
uniform vec3 light;
uniform float shine;
uniform vec3 Kd;
uniform vec3 Ks;
uniform vec3 Ka;
uniform mat4 V;
uniform vec4 Lc;
out vec4 color;

uniform sampler2DShadow shadowTex;

vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

// Random function taken from opengl-tutorial.org
float random(vec3 seed, int i){
    vec4 seed4 = vec4(seed,i);
    float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}

float shadowing() {
    vec3 view_light = (V * vec4(light, 1.0f)).xyz;
    vec3 light_vec = normalize(view_light - position);
    vec3 fragNor = normalize(fragNorVert);

    float vis = 1.0;
    float bias = 0.005 * tan(acos(dot(fragNor,light_vec)));
    bias = clamp(bias, 0.005, 0.05);

    vec3 projCoords = ShadowCoord.xyz / ShadowCoord.w;
    projCoords = vec3(projCoords.xy * 0.5 + 0.5, projCoords.z);

    if(projCoords.x < 0 || projCoords.x > 1 || projCoords.y < 0 ||
       projCoords.y > 1) {
        return 1.0;
    }
    for(int i = 0; i<4; i++) {
        int index = int(16.0*random(floor(position*1000.0), i))%16;
        vec3 texelCoords = vec3(projCoords.xy + poissonDisk[index]/700.0,
                                projCoords.z-bias);
        vis -= 0.2 * (1.0 - texture(shadowTex, texelCoords));
    }

    return vis;
}

void main()
{
    vec3 view_light = (V * vec4(light, 1.0f)).xyz;
    vec3 light_vec = normalize(view_light - position);
    vec3 fragNor = normalize(fragNorVert);
    vec3 view_dir = normalize(-position);
    vec3 R = reflect(-light_vec, fragNor);

    vec3 specular = Ks * pow(max(0, dot(view_dir, R)), shine);
    vec3 difuse = Kd * max(0, dot(light_vec, fragNor));
    vec3 ambient = Ka;

    float vis = shadowing();
    color = vec4(vis * (specular + difuse) + ambient, 1.0f) * Lc;
    //color = vec4(vis);
}
