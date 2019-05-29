#include "Texture.h"
#include "FreeImage\FreeImage.h"
#include <iostream>

Texture::Texture(const std::string& path)
{
	glGenTextures(1, &handle);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, handle);

	unsigned int width, height;

	// taken somewhat from http://www.codeproject.com/Questions/338143/While-Using-FreeImage-PNG-Image-Loads-and-Binds-bu
	BYTE* bits(0);

	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	fif = FreeImage_GetFileType(path.c_str(), 0);

	FIBITMAP *pic = FreeImage_Load(fif, path.c_str());

	if (!pic)
	{
		std::cout << "Failed to load image: " << path << "\n";
		bits = 0;
	}

	pic = FreeImage_ConvertTo32Bits(pic);

	width = FreeImage_GetWidth(pic);
	height = FreeImage_GetHeight(pic);

	//std::cout << "height: " << height << ", width: " << width << "\n";

	bits = FreeImage_GetBits(pic);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bits);
	
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	FreeImage_Unload(pic);
}


Texture::~Texture()
{
	glDeleteTextures(1, &handle);
}

void Texture::bind(int unit)
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, handle);
}