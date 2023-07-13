#pragma once

#include "SphRenderer.h"

class SolidRenderer final : public SphRenderer
{
public:

	SolidRenderer();

	void setup(int numMeshes, int totalFrames, const std::string &root);

	virtual void update() override;

	virtual void draw(const glm::mat4 &viewMatrix, const glm::mat4 &projectMatrix,
		const glm::vec3 &lightPos, const glm::vec3 &lightColor, const glm::vec3 &cameraPos) override;

	void setDraw(bool draw) { m_draw = draw; }

	void setReset(bool reset) { m_reset = reset; }
	
	void pushColor(glm::vec3 color) { m_color.emplace_back(color); }

private:
	bool							readMeshData(int frameId);
	bool							m_draw = false;
	bool							m_reset = false;
	

private:
	ofShader									 m_solidShader;
	std::vector<std::unique_ptr<ofVec3f[]>>      m_vertices;
	std::vector<std::unique_ptr<ofVec3f[]>>		 m_normals;
	std::vector<int>							 m_numVertices;
	std::vector<glm::vec3>						 m_color;

};