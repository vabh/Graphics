#version 330 core

// Values that stay constant for all fragments
uniform vec3 lightDirection; //direction of the light
uniform int specularE;
uniform int cartoon_mode;

// Interpolated values from the vertex shaders
in vec3 fragmentNormal;
in vec3 viewDirection;

in vec2 fragmentUV;
in vec3 v_position;

uniform sampler2D myTextureSampler;

// Ouput data
out vec3 color;

void main()
{      
      //Light direction
      vec3 L = normalize(lightDirection);

      // normal is interpolated between all 3 surrounding vertices, it needs to be normalized
      vec3 N = normalize(fragmentNormal);

      //view direction
      vec3 V =  normalize(viewDirection);

      //reflection direction      
      vec3 R = reflect(L, N);

      float diffuse = max(0, dot(N, -L));

      float specular = max(0, dot(V, R));
      specular = pow(specular, specularE);

      if(cartoon_mode == 1){

            if (diffuse < 0.4)
                  diffuse = 0.5;
            else 
                  diffuse = 1;

            if (specular < 0.1)
                  specular = 0;
            else 
                  specular = 1;
      }                   

      if(cartoon_mode == 1 && dot(N,V) <= 0.1)
            color = vec3(0,0,0);
      else
            color = diffuse * vec3(0.2, 0.2, 0.9) + specular * vec3(0.9, 0.9, 0.9);

      //TP3
      vec4 tex_data = texture(myTextureSampler, fragmentUV);
      
      color = vec3(tex_data.x, tex_data.y, tex_data.z);
      
      if(tex_data.x == 1)
            color =  diffuse * vec3(0.6, 0.1, 0.1);
      else
            color = diffuse * vec3(0.9, 0.5, 0.0) + specular * vec3(0.9, 0.5, 0.0);

      float x = v_position.x;
      float y = v_position.y;
      float z = v_position.z;

      float r = sqrt(pow(x,2) + pow(y,2) + pow(z,2));

      if(r <= 1 && r >= 0.5)
            //color = normalize(vec3(r*x,r*y,r*z));
            color =  diffuse * vec3(0.6, 0.1, 0.1);
      else if(r <= 1.5 && r >= 1)
            color = diffuse * vec3(0.9, 0.5, 0.0) + specular * vec3(0.9, 0.5, 0.0);
      else
            color =  diffuse * vec3(0.6, 0.6, 0.6);

      float r1 = sin(r);
      if(r1 <= 0.5 || r1 >= 0.6)
            color = vec3(1,1,1);
      else color = vec3(0,0,0);
      
}     