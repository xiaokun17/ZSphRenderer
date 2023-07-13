#include "Cubemap.h"

#include "ofGLUtils.h"

Cubemap::Cubemap()
{
	glEnable(GL_TEXTURE_CUBE_MAP);
	m_textureData.texData.bAllocated = false;
	m_textureData.texData.glInternalFormat = GL_RGB;
	m_textureData.texData.textureID = 0;
	m_textureData.texData.textureTarget = GL_TEXTURE_CUBE_MAP;
}

Cubemap::~Cubemap()
{
	glDeleteTextures(1, &m_glTexId);
}

bool Cubemap::load(
	const std::string& front,
	const std::string& back,
	const std::string& right,
	const std::string& left,
	const std::string& top,
	const std::string& bottom)
{

	bool success = m_images[0].load(right);
	success |= m_images[1].load(left);
	success |= m_images[2].load(top);
	success |= m_images[3].load(bottom);
	success |= m_images[4].load(front);
	success |= m_images[5].load(back);

	if (!success)
	{
		fprintf(stderr, "ERROR: Cubemap failed to load an image");
		return false;
	}

	unsigned int faceWidth = m_images[0].getWidth();
	unsigned int faceHeight = m_images[0].getHeight();

	glGenTextures(1, &m_glTexId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_glTexId);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	unsigned char* faceData[6];

	for (int i = 0; i < 6; ++i)
	{
		if (m_images[i].getWidth() != faceWidth || m_images[i].getHeight() != faceHeight)
		{
			fprintf(stderr, "ERROR: Cubemap couldn't load because not all source textures are the same size\n");
			return false;
		}

		faceData[i] = m_images[i].getPixels().getData();

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_images[i].getTexture().texData.glInternalFormat,
			faceWidth, faceHeight, 0, ofGetGLFormat(m_images[i].getPixels()), GL_UNSIGNED_BYTE, faceData[i]);
	}
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	m_textureData.texData.textureID = m_glTexId;
	m_textureData.texData.bAllocated = true;

	return true;
}

const ofTexture& Cubemap::getTexture() const
{
	return m_textureData;
}

ofTexture& Cubemap::getTexture()
{
	return m_textureData;
}