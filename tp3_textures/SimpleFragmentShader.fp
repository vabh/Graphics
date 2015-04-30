#version 330 core

// Values that stay constant for all fragments
uniform vec3 lightDirection; //direction of the light
// Texture sampler
uniform sampler2D myTextureSampler;

// Interpolated values from the vertex shaders
in vec3 fragmentNormal;
in vec3 viewDirection;
// Interpolated texture coordinates [Q1c]


// Ouput data
out vec3 color;


void main()
{
      vec3 L = normalize(lightDirection);

      // normal is interpolated between all 3 surrounding vertices, it needs to be normalized before shading computation
      vec3 N = normalize(fragmentNormal);

      // Diffuse shading
      float diffuse = max(dot(N,-L),0);
      color = vec3(diffuse,diffuse,diffuse);
}