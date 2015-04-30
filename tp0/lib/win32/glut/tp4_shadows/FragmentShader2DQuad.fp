#version 330 core

// Interpolated values from the vertex shaders
in vec2 fragmentUV;

// Values that stay constant for the whole mesh
uniform sampler2D myTextureSampler;

// Ouput data
out vec3 color;

void main()
{
      color = texture(myTextureSampler, fragmentUV).rgb;
}