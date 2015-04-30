#version 330 core

// Interpolated values from the vertex shaders
in vec3 fragmentNormal;
in vec4 vertexPosition_lightspace;

uniform sampler2D shadowTexture;
uniform vec3 lightDirection;

// Ouput data
out vec3 color;

void main()
{
      //vec4 lightDirection = vec4(-3.,3.,-2.,1); 
      vec3 L = normalize(lightDirection.xyz);

      // normal is interpolated between all 3 surrounding vertices, it needs to be normalized
      vec3 N = normalize(fragmentNormal);

      // Diffuse shading
      float diffuse = max(dot(N,-L),0.);

      vec2 poissonDisk[4] = vec2[](
							  vec2( -0.94201624, -0.39906216 ),
							  vec2( 0.94558609, -0.76890725 ),
							  vec2( -0.094184101, -0.92938870 ),
							  vec2( 0.34495938, 0.29387760 )
							);

      //vec3 LPos = normalize(vertexPosition_lightspace.xyz);      
      vec3 LPos = vertexPosition_lightspace.xyz; 
      vec2 uv = vec2((LPos.x + 1) / 2, (LPos.y + 1) / 2);
      vec4 texData = texture(shadowTexture, uv);

      float z2 = texData.z; 
      float z1 = (LPos.z + 1) / 2;

      float bias = 0.005;

      	float visibility = 1.0;

      	//Poisson Sampling
	  	for (int i=0;i<4;i++){	  		
		  	if ( texture( shadowTexture, uv + poissonDisk[i]/700.0 ).z  <  z1 - bias ){
		    	visibility-=0.2;
		  	}
		}

		//stratified Poisson sampling
		//after I enable this line, I notice bands in the image		
		for (int i=0;i<4;i++){
	  		float dot_product = dot(gl_FragCoord, vec4(12.9898,78.233,45.164,94.673)) * 16;
    		float index = fract(sin(dot_product) * 43758.5453) * 16;
    		int index1 = int(mod(index, 4));
		  	//visibility -= 0.2 * (texture( shadowTexture, uv + poissonDisk[index1]/640.0)).z;
		}
		

      if(z2 < z1 - bias )
      	color = visibility * texData.rgb;       
      else
      	color = visibility * diffuse * vec3(0,0.5,0.3);
}