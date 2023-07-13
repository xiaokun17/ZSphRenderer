#pragma once

#include "ofMain.h"
#include <memory>

class SphRenderer
{
public:

	SphRenderer() = default;
	virtual ~SphRenderer() = default;

	virtual void update() = 0;

	virtual void draw(const glm::mat4 &viewMatrix, const glm::mat4 &projectMatrix,
		const glm::vec3 &lightPos, const glm::vec3 &lightColor, const glm::vec3 &cameraPos) = 0;

protected:
	std::vector<ofVbo>					m_vbos;
	std::string							m_rootPath;
	static std::vector<unsigned char>	m_readBuffer;

	int									m_totalFrames = 1;
	int									m_currentFrame = 1;

};