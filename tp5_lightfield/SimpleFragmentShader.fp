#version 330 core

// Interpolated values from the vertex shaders
in vec2 fragmentUV;

// Values that stay constant for the whole mesh
uniform sampler2D myTextureSampler;

uniform int mouseX;
uniform int mouseY;
uniform float alpha;

// Ouput data
out vec3 color;

void main()
{ 
      //color = vec3(fragmentUV,0.);
      //texture co ordinates are between 0, 1
      //movement based on mouse location
      float x = fragmentUV.x / 9 + mouseX / 9.;
      float y = fragmentUV.y / 9 + mouseY / 9.;

      //Q2
      color = vec3(0, 0, 0);
      for(int i = 0; i < 9; i++){
      	for(int j = 0 ; j < 9; j++){

      		//texture co ordinates are between 0, 1
      		float x1 = fragmentUV.x / 9. + i / 9.;
      		float y1 = fragmentUV.y / 9. + j / 9.;

      		color = color + texture(myTextureSampler, vec2(x1, y1)).rgb;
      	}
      }

      //Q3      
      vec3 color2 = vec3(0, 0, 0);
      int i0 = 4;
      int j0 = 4;
      for(int i = 0; i <= 9; i++){
            for(int j = 0 ; j <= 9; j++){
                  
                  float translationX = alpha * (i - i0);
                  float translationY = alpha * (j - j0);

                  float x2 = (fragmentUV.x / 9. + i/9. + translationX);
                  float y2 = (fragmentUV.y / 9. + j/9. + translationY);

                  color2 = color2 + texture(myTextureSampler, vec2(x2, y2)).rgb;
            }
      }

      //Q1
      //color = texture(myTextureSampler, vec2(x, y)).rgb;

      //Q2, Q3  
      //color = color / 81.;
      color = color2 / 81.;      
}