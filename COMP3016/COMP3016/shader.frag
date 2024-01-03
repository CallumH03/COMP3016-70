#version 330 core
in vec2 TexCoord;

uniform sampler2D ourTexture;  
uniform int objectType;         

out vec4 FragColor;

void main() {
    if (objectType == 0) {
        FragColor = vec4(0.0, 0.0, 1.0, 1.0);  // blue color for rain
    } else {
        FragColor = texture(ourTexture, TexCoord); //texture for plane
    }
}