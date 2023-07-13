#pragma once

#include "ofMain.h"

class MeshRenderer final
{
public:

	MeshRenderer();

	void setup(const std::string &path);

	void update();

	void draw(const glm::mat4 &viewMatrix, const glm::mat4 &projectMatrix,
		const glm::vec3 &lightPos, const glm::vec3 &lightColor, const glm::vec3 &cameraPos);

private:
	std::vector<ofVbo>							 m_vbos;
	std::vector<ofTexture>						 m_texs;
	std::vector<int>							 m_numIndices;
	ofShader									 m_meshShader;
};