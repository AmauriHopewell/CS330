#version 330 core

in vec2 TexCoord;  // Interpolated UV from vertex shader

uniform vec4 objectColor;          // Solid color (from SetShaderColor)
uniform sampler2D objectTexture;   // First texture (e.g., "clockface" for bottom)
uniform sampler2D objectTexture2;  // Second texture (e.g., "knobTexture" for top)
uniform int bUseTexture;           // Flag: 1 = use texture, 0 = use color
uniform int bUseTwoTextures;       // Flag: 1 = split with two textures

out vec4 FragColor;  // Final pixel color

void main() {
    vec4 color;
    
    if (bUseTwoTextures == 1) {
        // Split at v=0.5: bottom half (v <= 0.5) uses objectTexture ("clockface")
        // Top half (v > 0.5) uses objectTexture2 ("knobTexture")
        if (TexCoord.y > 0.5) {
            color = texture(objectTexture2, TexCoord);
        } else {
            color = texture(objectTexture, TexCoord);
        }
    } else if (bUseTexture == 1) {
        // Single texture mode
        color = texture(objectTexture, TexCoord);
    } else {
        // Solid color mode
        color = objectColor;
    }
    
    FragColor = color;  // Output (could multiply by lighting if added later)
}