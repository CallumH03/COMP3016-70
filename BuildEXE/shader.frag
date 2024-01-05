#version 330 core
in vec2 TexCoord; //input variable from vertex shader

uniform sampler2D Texture; //texture sampler 
uniform int objectType; //variable for determing texture/color  

out vec4 FragColor; //color variable

void main() {
    if (objectType == 0) {
        FragColor = vec4(0.0, 0.0, 1.0, 1.0);  // Blue color for rain
    } else if (objectType == 1) {
        FragColor = texture(Texture, TexCoord); // grass/bush texture
    } else if (objectType == 2) {
        vec4 textureColor = texture(Texture, TexCoord);
        vec4 solidColor = vec4(0.0, 0.4, 0.0, 1.0); // Dark Green color
        FragColor = textureColor * solidColor;
    }
}