#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 worldNormal;
in vec3 fragVert;

// Ouput data
layout(location=0) out vec4 outColor;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;
uniform vec3 camera_position;
uniform mat4 model;

#define MAX_LIGHTS 10
#define AMBIENT_COEFF 0.5
#define BASE_ATTENUATION 0.4

uniform int numLights;
uniform struct Light {
   vec3 position;
   vec3 intensities;
} lights[MAX_LIGHTS];

void main(){

	//outColor = texture2D( myTextureSampler, UV ).bgr;

	// some code taken from http://www.tomdalling.com/blog/modern-opengl/06-diffuse-point-lighting/
		// and http://www.tomdalling.com/blog/modern-opengl/08-even-more-lighting-directional-lights-spotlights-multiple-lights/
		// cel-shading: http://prideout.net/blog/?p=22#downloads
		// outlines: https://en.wikibooks.org/wiki/GLSL_Programming/Unity/Toon_Shading#Outlines

		//calculate normal in world coordinates
		vec3 normal = normalize(worldNormal);
    
		//calculate the location of this fragment (pixel) in world coordinates
		vec3 fragPosition = vec3(model * vec4(fragVert, 1));

		//calculate view direction
		vec3 viewDirection = normalize(camera_position - vec3(fragPosition));

		// ambient base lightning
		vec3 color = vec3((AMBIENT_COEFF * texture(myTextureSampler, UV).bgr));

		const float A = 0.1;
		const float B = 0.3;
		const float C = 0.6;
	    const float D = 1.0;

		for(int i = 0; i < numLights; i++)
		{
			//calculate the vector from this pixels surface to the light source
			vec3 surfaceToLight = (lights[i].position - fragPosition);

			float attenuation = 1.0 / (1.0 + BASE_ATTENUATION * (length(lights[i].position - fragPosition)));

			//calculate the cosine of the angle of incidence
			float brightness = max(0.0, dot(normal, surfaceToLight));

			// Cel-Shader
			if (brightness < A) brightness = 0.0;
			else if (brightness < B) brightness = B;
			else if (brightness < C) brightness = C;
			else brightness = D;
			
			//calculate final color of the pixel
			// + diffuse
			color += attenuation * brightness * lights[i].intensities * texture(myTextureSampler, UV).bgr;
		}
		
		//calculate outlines
		if (dot(viewDirection, normal) < 0.3)
        {
			outColor = vec4(color.xyz, texture(myTextureSampler, UV).a) * vec4(0,0,0,1); 
        }
		else {
			outColor = vec4(color.xyz, texture(myTextureSampler, UV).a);
		}

}

