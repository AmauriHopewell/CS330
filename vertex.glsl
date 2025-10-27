#version 330 core

layout (location = 0) in vec3 aPosition;  // Vertex position from mesh
layout (location = 1) in vec2 aTexCoord;  // UV texture coordinates from mesh (u horizontal, v vertical)

uniform mat4 model;       // Model matrix (from C++ SetTransformations)
uniform mat4 view;        // View matrix (camera)
uniform mat4 projection;  // Projection matrix

out vec2 TexCoord;  // Passed to fragment shader

void main() {
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
    TexCoord = aTexCoord;  // Forward UVs (v=0.0 at bottom, v=1.0 at top for standard sphere)
}