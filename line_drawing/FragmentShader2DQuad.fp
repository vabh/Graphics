#version 330 core

// Interpolated values from the vertex shaders
in vec2 fragmentUV;

// Values that stay constant for the whole mesh
uniform sampler2D myTextureSampler;
uniform int width;
uniform int height;

// Ouput data
out vec3 color;

void main()
{
      //color = texture(myTextureSampler, fragmentUV).rgb;
     	vec3 val1 = texture(myTextureSampler, fragmentUV - 1.0 / width - 1.0 / height).rgb;
		vec3 val2 = texture(myTextureSampler, fragmentUV + 1.0 / width - 1.0 / height).rgb;
		vec3 val3 = texture(myTextureSampler, fragmentUV - 1.0 / width + 1.0 / height).rgb;
		vec3 val4 = texture(myTextureSampler, fragmentUV + 1.0 / width + 1.0 / height).rgb;
		vec3 edgeX = 4.0 * ((val4 + val3) - (val2 + val1));
		vec3 edgeY = 4.0 * ((val2 + val4) - (val1 + val3));
		vec3 sobel = sqrt(edgeX * edgeX + edgeY * edgeY);
		color = 1. - sobel;
}