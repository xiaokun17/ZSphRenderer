#pragma once

#include "ofMain.h"
#include "Cubemap.h"

#include <memory>

class SkyRenderer final
{
public:

	SkyRenderer();

	void setup(const std::string &rootPath);

	void update();

	void draw(const glm::mat4 &viewMatrix, const glm::mat4 &projectMatrix,
		const glm::vec3 &lightPos, const glm::vec3 &lightColor, const glm::vec3 &cameraPos);

	ofTexture &getCubemap() const { return m_cubemap->getTexture(); }

private:
	std::unique_ptr<Cubemap>					 m_cubemap = nullptr;
	ofShader									 m_skyShader;
	ofVbo										 m_vbo;
};
