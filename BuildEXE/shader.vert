#version 330 core
layout (location = 0) in vec3 aPos; //vertex position
layout (location = 1) in vec2 aTexCoord; //texture coordinates
out vec2 TexCoord; //variable for fragment shader

//matrixes
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0); //combine to transform vertex position
    TexCoord = aTexCoord; //pass texture coordinate to frag shader
}