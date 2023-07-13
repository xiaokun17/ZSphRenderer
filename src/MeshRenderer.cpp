#include "MeshRenderer.h"

#include "ofxAssimpModelLoader.h"

MeshRenderer::MeshRenderer()
{
	m_meshShader.load("shaders/mesh.vert", "shaders/mesh.frag");
}

void MeshRenderer::setup(const std::string &path)
{
	static ofxAssimpModelLoader model;
	model.loadModel(path, true);
	
	unsigned int numMeshes = model.getNumMeshes();
	m_vbos.resize(numMeshes);
	m_texs.resize(numMeshes);
	m_numIndices.resize(numMeshes);
	for (unsigned int i = 0; i < numMeshes; ++i)
	{
		m_texs[i] = model.getTextureForMesh(i);
		m_texs[i].generateMipmap();
		m_texs[i].enableMipmap();
		m_texs[i].setTextureWrap(GL_REPEAT, GL_REPEAT);
		const auto &mesh = model.getMesh(i);
		const auto &verts = mesh.getVertices();
		const auto &norms = mesh.getNormals();
		const auto &texcoords = mesh.getTexCoords();
		const auto &indices = mesh.getIndices();
		m_vbos[i].setVertexData(verts.data(), verts.size(), GL_STATIC_DRAW);
		m_vbos[i].setNormalData(norms.data(), norms.size(), GL_STATIC_DRAW);
		m_vbos[i].setTexCoordData(texcoords.data(), texcoords.size(), GL_STATIC_DRAW);
		m_vbos[i].setIndexData(indices.data(), indices.size(), GL_STATIC_DRAW);
		m_numIndices[i] = mesh.getNumIndices();
	}
	
}

void MeshRenderer::update()
{
	//nothing
}

void MeshRenderer::draw(const glm::mat4 &viewMatrix, const glm::mat4 &projectMatrix,
	const glm::vec3 &lightPos, const glm::vec3 &lightColor, const glm::vec3 &cameraPos)
{
	m_meshShader.begin();

	m_meshShader.setUniform3f("u_light.position", lightPos);
	m_meshShader.setUniform3f("u_light.lightColor", lightColor);

	m_meshShader.setUniformMatrix4f("u_viewMatrix", viewMatrix);
	m_meshShader.setUniformMatrix4f("u_projectMatrix", projectMatrix);
	m_meshShader.setUniform3f("u_cameraPos", cameraPos);

	for (unsigned int i = 0; i < m_vbos.size(); ++i)
	{
		m_meshShader.setUniformTexture("image", m_texs[i], 0);
		m_vbos[i].drawElements(GL_TRIANGLES, m_numIndices[i]);
	}

	m_meshShader.end();
}