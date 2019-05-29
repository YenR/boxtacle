#version 330 core

#define MAX_LIGHTS 5
#define AMBIENT_COEFF 0.2
#define BASE_ATTENUATION 0.4

layout(location=0) out vec4 outColor;

in vec3 worldNormal;
in float dmg;
in vec2 texCoord;
in vec3 fragVert;

in vec4 ShadowCoord[MAX_LIGHTS];

uniform mat4 model;
uniform vec4 materialColor;


uniform int numLights;
uniform struct Light {
   vec3 position;
   vec3 intensities;
} lights[MAX_LIGHTS];

uniform int simpleDraw;
uniform sampler2DShadow shadowMap[MAX_LIGHTS];

void main()
{
	if(simpleDraw == 0)
	{
		if( texCoord.x < 0.01 || texCoord.x > 0.99 || texCoord.y < 0.01 || texCoord.y > 0.99)
		{
			outColor = vec4(0,0,0, materialColor.a);
		}
		else
		{
			// some code taken from http://www.tomdalling.com/blog/modern-opengl/06-diffuse-point-lighting/
			// and http://www.tomdalling.com/blog/modern-opengl/08-even-more-lighting-directional-lights-spotlights-multiple-lights/

			//calculate normal in world coordinates
			vec3 normal = normalize(worldNormal);
    
			//calculate the location of this fragment (pixel) in world coordinates
			vec3 fragPosition = vec3(model * vec4(fragVert, 1));

			// ambient base lightning
			vec3 color = vec3((AMBIENT_COEFF * materialColor.rgb));
			color.r += dmg;
			color.g -= dmg;
			color.b -= dmg;

			const float A = 0.1;
			const float B = 0.3;
			const float C = 0.6;
			const float D = 1.0;

			for(int i = 0; i < numLights; i++)
			{
				float shadow_factor = 1;
				vec3 coords = ShadowCoord[i].xyz / ShadowCoord[i].w;

				if(coords.x >= 0.001 && coords.x < 0.999 &&
					coords.y >= 0.001 && coords.y < 0.999 &&
					coords.z >= 0.001 && coords.z < 0.999)
				{
					if ( texture(shadowMap[i+1], coords)  <  coords.z)
					{
						shadow_factor = 0.0f;
					}
				}

					//calculate the vector from this pixels surface to the light source
					vec3 surfaceToLight = normalize(lights[i].position - fragPosition);

					float attenuation = 1.0 / (1.0 + BASE_ATTENUATION * (length(lights[i].position - fragPosition)));

					//calculate the cosine of the angle of incidence
					float brightness = max(0.0, dot(normal, surfaceToLight));
					if (brightness < A) brightness = 0.0;
					else if (brightness < B) brightness = B;
					else if (brightness < C) brightness = C;
					else brightness = D;
					//calculate final color of the pixel
					// + diffuse
					
					color += shadow_factor * attenuation * brightness * lights[i].intensities * materialColor.rgb;
			}

			outColor = vec4(color.xyz, materialColor.a);
		}
	}
}