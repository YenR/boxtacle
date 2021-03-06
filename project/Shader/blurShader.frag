#version 330 core

layout(location=0) out vec4 outColor;

in vec3 worldNormal;
in vec2 texCoord;
in vec2 image_size;

uniform sampler2D color_texture;

#define LENSFLARE_INTENSITY 0.04
#define LENSFLARE_NUM_GHOSTS 4
#define LENSFLARE_DISPERSION 0.5

void main()
{
	
    vec4 summ;

	// Gaussian Filter generated by http://www.embege.com/gauss/ (5x5, variance = 3, normalized)		

	float weights1[5] = float[](0.01905031014488527, 0.03140865154930652, 0.03710493756184187, 0.03140865154930652, 0.01905031014488527);
	float weights2[5] = float[](0.03140865154930652, 0.05178411189334978, 0.06117569980620832, 0.05178411189334978, 0.03140865154930652);
	float weights3[5] = float[](0.03710493756184187, 0.06117569980620832, 0.07227054998040688, 0.06117569980620832, 0.03710493756184187);
	float weights4[5] = float[](0.03140865154930652, 0.05178411189334978, 0.06117569980620832, 0.05178411189334978, 0.03140865154930652);
	float weights5[5] = float[](0.01905031014488527, 0.03140865154930652, 0.03710493756184187, 0.03140865154930652, 0.01905031014488527);


	if(texCoord.x > 0.01 && texCoord.x < 0.99 && texCoord.y > 0.01 && texCoord.y < 0.99)
	{
		for(int i = 0; i < 5; i++)
		{
			summ += texture2D(color_texture, texCoord + vec2( (float(i - 2))/image_size.x , (float(-2))/image_size.y)) * weights1[i];
			
			summ += texture2D(color_texture, texCoord + vec2( (float(i - 2))/image_size.x , (float(-1))/image_size.y)) * weights2[i];
			
			summ += texture2D(color_texture, texCoord + vec2( (float(i - 2))/image_size.x , (float(0))/image_size.y)) * weights3[i];
			
			summ += texture2D(color_texture, texCoord + vec2( (float(i - 2))/image_size.x , (float(1))/image_size.y)) * weights4[i];
			
			summ += texture2D(color_texture, texCoord + vec2( (float(i - 2))/image_size.x , (float(2))/image_size.y)) * weights5[i];	
		}
		
		// Create Lensflares (Pseudo Lensflares as per http://john-chapman-graphics.blogspot.co.at/2013/02/pseudo-lens-flare.html)
		vec2 texcoord = -texCoord + vec2(1.0);
		vec2 texelSize = 1.0 / vec2(textureSize(color_texture, 0));
 
		// ghost vector to image centre:
		vec2 ghostVec = (vec2(0.5) - texcoord) * LENSFLARE_DISPERSION;
   
		// sample ghosts:  
		vec4 result = vec4(0.0);
		for (int i = 0; i < LENSFLARE_NUM_GHOSTS; ++i) 
		{ 
			vec2 offset = (texcoord + ghostVec * float(i));
			
			float weight = length(vec2(0.5) - offset) / length(vec2(0.5));
			weight = pow(1.0 - weight, 10.0);
	
	
			for(int i = 0; i < 5; i++)
			{
				result += texture2D(color_texture, offset + vec2( (float(i - 2))/image_size.x , (float(-2))/image_size.y)) * weights1[i];
			
				result += texture2D(color_texture, offset + vec2( (float(i - 2))/image_size.x , (float(-1))/image_size.y)) * weights2[i];
			
				result += texture2D(color_texture, offset + vec2( (float(i - 2))/image_size.x , (float(0))/image_size.y)) * weights3[i];
			
				result += texture2D(color_texture, offset + vec2( (float(i - 2))/image_size.x , (float(1))/image_size.y)) * weights4[i];
			
				result += texture2D(color_texture, offset + vec2( (float(i - 2))/image_size.x , (float(2))/image_size.y)) * weights5[i];	
			}

			result *= weight;
		}
      
		outColor = vec4(1, 1, 1, (summ.r + summ.g + summ.b) * 0.2) + vec4(0, 0, 0, (result.r + result.g + result.b) * LENSFLARE_INTENSITY) ;
			
	}
	else
	{
		outColor = vec4(texture2D(color_texture, texCoord).rgb, 0);
	}


}