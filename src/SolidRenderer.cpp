#include "SolidRenderer.h"

SolidRenderer::SolidRenderer()
{
	m_solidShader.load("shaders/solid.vert", "shaders/solid.frag");
}

void SolidRenderer::setup(int numMeshes, int totalFrames, const std::string &root)
{
	m_totalFrames = totalFrames;
	m_rootPath = root;

	m_vbos.clear();
	m_vertices.clear();
	m_normals.clear();
	m_numVertices.clear();

	if (numMeshes > 0)
	{
		m_vbos.resize(numMeshes);
		m_vertices.resize(numMeshes);
		m_normals.resize(numMeshes);
		m_numVertices.resize(numMeshes);

		for (int i = 0; i < numMeshes; ++i)
		{
			m_vertices[i] = nullptr;
			m_normals[i] = nullptr;
		}

	}

	m_currentFrame = 1;
}

void SolidRenderer::update()
{
	if (m_vbos.empty() || m_totalFrames <= 0)
		return;

	if (m_reset)
	{
		m_currentFrame = 1;
		m_reset = false;
	}

	if (m_draw)
	{
		m_currentFrame += 1;
		if (m_currentFrame == m_totalFrames + 1)
			m_currentFrame = 1;
	}

	readMeshData(m_currentFrame);
}

void SolidRenderer::draw(const glm::mat4 &viewMatrix, const glm::mat4 &projectMatrix,
	const glm::vec3 &lightPos, const glm::vec3 &lightColor, const glm::vec3 &cameraPos)
{

	if (m_vbos.empty())
		return;

	m_solidShader.begin();

	m_solidShader.setUniform3f("u_light.position", lightPos);
	m_solidShader.setUniform3f("u_light.lightColor", lightColor);

	m_solidShader.setUniformMatrix4f("u_viewMatrix", viewMatrix);
	m_solidShader.setUniformMatrix4f("u_projectMatrix", projectMatrix);

	m_solidShader.setUniform3f("u_cameraPos", cameraPos);

	for (int i = 0; i < m_vbos.size(); ++i)
	{
		m_solidShader.setUniform3f("f_color", m_color[i]);
		m_vbos[i].draw(GL_TRIANGLES, 0, m_numVertices[i]);
	}

	m_solidShader.end();
}

bool SolidRenderer::readMeshData(int frameId)
{
	std::stringstream ss;
	ss << "SolidFrame/frame." << setfill('0') << setw(4) << frameId << ".pos";

	std::string filename = m_rootPath + ss.str();

	std::ifstream file(filename.c_str(), std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		std::cerr << "Failed to load the file: " << filename << std::endl;
		return false;
	}

	size_t fileSize = (size_t)file.tellg();
	m_readBuffer.resize(fileSize);
	file.seekg(0, std::ios::beg);
	file.read((char*)m_readBuffer.data(), fileSize);
	file.close();

	//Number of meshes
	int numMeshes = 0;
	memcpy(&numMeshes, m_readBuffer.data(), sizeof(int));

	size_t offset = sizeof(int);
	size_t dataSize = sizeof(int);

	for (int i = 0; i < numMeshes; ++i) 
	{
		int numVertices;
		dataSize = sizeof(int);

		memcpy(&numVertices, &m_readBuffer.data()[offset], dataSize);
		offset += dataSize;

		dataSize = 3 * numVertices * sizeof(float);
		assert(dataSize * 2 + offset <= m_readBuffer.size());

		if (m_vertices[i] == nullptr || m_normals[i] == nullptr)
		{
			//First loading
			m_numVertices[i] = numVertices;
			m_vertices[i].reset(new ofVec3f[numVertices]);
			m_normals[i].reset(new ofVec3f[numVertices]);

			memcpy(m_vertices[i].get(), (void*)&m_readBuffer.data()[offset], dataSize);
			offset += dataSize;
			memcpy(m_normals[i].get(), (void*)&m_readBuffer.data()[offset], dataSize);
			offset += dataSize;
			
			m_vbos[i].setVertexData(m_vertices[i].get(), numVertices, GL_DYNAMIC_DRAW);
			m_vbos[i].setNormalData(m_normals[i].get(), numVertices, GL_DYNAMIC_DRAW);
		}
		else
		{
			//Update loading
			assert(m_numVertices[i] == numVertices);
			memcpy(m_vertices[i].get(), (void*)&m_readBuffer.data()[offset], dataSize);
			offset += dataSize;
			memcpy(m_normals[i].get(), (void*)&m_readBuffer.data()[offset], dataSize);
			offset += dataSize;

			m_vbos[i].updateVertexData(m_vertices[i].get(), numVertices);
			m_vbos[i].updateNormalData(m_normals[i].get(), numVertices);
		}
	}

	return true;
}