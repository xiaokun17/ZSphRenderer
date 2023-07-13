#include "SkyRenderer.h"

SkyRenderer::SkyRenderer()
{
	m_skyShader.load("shaders/sky.vert", "shaders/sky.frag");
}

void SkyRenderer::setup(const std::string &rootPath)
{
	const std::string front = rootPath + "negz.png";
	const std::string back = rootPath + "posz.png";
	const std::string right = rootPath + "negx.png";
	const std::string left = rootPath + "posx.png";
	const std::string top = rootPath + "posy.png";
	const std::string bottom = rootPath + "negy.png";
	
	m_cubemap.reset(new Cubemap());
	m_cubemap->load(front, back, right, left, top, bottom);

	ofVec3f skyboxVertices[] = 
	{
		ofVec3f(-1.0f,  1.0f, -1.0f),
		ofVec3f(-1.0f, -1.0f, -1.0f),
		ofVec3f(1.0f, -1.0f, -1.0f),
		ofVec3f(1.0f, -1.0f, -1.0f),
		ofVec3f(1.0f,  1.0f, -1.0f),
		ofVec3f(-1.0f,  1.0f, -1.0f),

		ofVec3f(-1.0f, -1.0f,  1.0f),
		ofVec3f(-1.0f, -1.0f, -1.0f),
		ofVec3f(-1.0f,  1.0f, -1.0f),
		ofVec3f(-1.0f,  1.0f, -1.0f),
		ofVec3f(-1.0f,  1.0f,  1.0f),
		ofVec3f(-1.0f, -1.0f,  1.0f),

		ofVec3f(1.0f, -1.0f, -1.0f),
		ofVec3f(1.0f, -1.0f,  1.0f),
		ofVec3f(1.0f,  1.0f,  1.0f),
		ofVec3f(1.0f,  1.0f,  1.0f),
		ofVec3f(1.0f,  1.0f, -1.0f),
		ofVec3f(1.0f, -1.0f, -1.0f),

		ofVec3f(-1.0f, -1.0f,  1.0f),
		ofVec3f(-1.0f,  1.0f,  1.0f),
		ofVec3f(1.0f,  1.0f,  1.0f),
		ofVec3f(1.0f,  1.0f,  1.0f),
		ofVec3f(1.0f, -1.0f,  1.0f),
		ofVec3f(-1.0f, -1.0f,  1.0f),

		ofVec3f(-1.0f,  1.0f, -1.0f),
		ofVec3f(1.0f,  1.0f, -1.0f),
		ofVec3f(1.0f,  1.0f,  1.0f),
		ofVec3f(1.0f,  1.0f,  1.0f),
		ofVec3f(-1.0f,  1.0f,  1.0f),
		ofVec3f(-1.0f,  1.0f, -1.0f),

		ofVec3f(-1.0f, -1.0f, -1.0f),
		ofVec3f(-1.0f, -1.0f,  1.0f),
		ofVec3f(1.0f, -1.0f, -1.0f),
		ofVec3f(1.0f, -1.0f, -1.0f),
		ofVec3f(-1.0f, -1.0f,  1.0f),
		ofVec3f(1.0f, -1.0f,  1.0f)
	};
	m_vbo.setVertexData(skyboxVertices, 36, GL_STATIC_DRAW);
}

void SkyRenderer::update()
{
	//nothing
}

void SkyRenderer::draw(const glm::mat4 &viewMatrix, const glm::mat4 &projectMatrix,
	const glm::vec3 &lightPos, const glm::vec3 &lightColor, const glm::vec3 &cameraPos)
{
	glDepthFunc(GL_LEQUAL);

	m_skyShader.begin();

	m_skyShader.setUniformTexture("cubemap", m_cubemap->getTexture(), 1);
	m_skyShader.setUniformMatrix4f("u_viewMatrix", glm::mat4(glm::mat3(viewMatrix)));
	m_skyShader.setUniformMatrix4f("u_projectMatrix", projectMatrix);

	m_vbo.draw(GL_TRIANGLES, 0, 36);

	m_skyShader.end();

	glDepthFunc(GL_LESS);
}