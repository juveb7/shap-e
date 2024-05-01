#version 330 core
layout (location = 0) in vec3 vertPos;  // Vertex position
layout (location = 1) in vec3 vertNor;  // Vertex normal (not used in this shader, but kept for compatibility)
layout (location = 2) in vec2 vertTex;  // Vertex texture coordinates

out vec2 TexCoords;  // Pass texture coordinates to fragment shader

uniform mat4 P;  // Projection matrix
uniform mat4 V;  // View matrix
uniform mat4 M;  // Model matrix

void main() {
    TexCoords = vertTex;  // Pass through texture coordinates
    gl_Position = P * V * M * vec4(vertPos, 1.0);  // Transform vertex position
}
