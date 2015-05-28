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
                  
      //distance from camera
      float xd = pow(v_position.x - viewDirection.x , 2);
      float yd = pow(v_position.y - viewDirection.y , 2);
      float zd = pow(v_position.z - viewDirection.z , 2);
      float dist = pow(xd + yd + zd, 0.5);

      color = vec3(0, 0, 0);
      for(float i = 0; i <= diffuse; i+=0.1){
           color += texture(myTextureSampler, vec2(fragmentUV.x + i/3., fragmentUV.y + 1)).rgb;
      }

      color = texture(myTextureSampler, vec2(fragmentUV.x / 3. + 0.3, fragmentUV.y / 3. + 0.3)).rgb;

      if(diffuse < 0.3)
            color = texture(myTextureSampler, vec2(fragmentUV.x / 3. + 0.6, fragmentUV.y / 3. + dist)).rgb;
      else if (diffuse >= 0.3 && diffuse < 0.6)
            color = texture(myTextureSampler, vec2(fragmentUV.x / 3. + 0.3, fragmentUV.y / 3. + dist)).rgb;
      else
            color = texture(myTextureSampler, vec2(fragmentUV.x / 3., fragmentUV.y / 3. + dist)).rgb;
      
}     