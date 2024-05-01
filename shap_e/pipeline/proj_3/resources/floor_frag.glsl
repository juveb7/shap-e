#version 330 core
in vec2 TexCoords;  // Texture coordinates from vertex shader
out vec4 FragColor;  // Output color

uniform sampler2D floorTex;  // Texture sampler for the floor

void main() {
    FragColor = texture(floorTex, TexCoords);  // Sample the texture at the passed coordinates
}
