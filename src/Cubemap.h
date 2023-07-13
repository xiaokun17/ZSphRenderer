#pragma once

#include "ofTexture.h"
#include "ofImage.h"

class Cubemap
{
public:
	Cubemap();
	~Cubemap();
	Cubemap(const Cubemap& other) = delete;
	Cubemap& operator=(const Cubemap& other) = delete;

	bool load(
		const std::string& front,
		const std::string& back,
		const std::string& right,
		const std::string& left,
		const std::string& top,
		const std::string& bottom);

	ofTexture& getTexture();
	const ofTexture& getTexture() const;

private:
	ofTexture m_textureData;
	unsigned int m_glTexId;
	ofImage m_images[6];

};