#include "LiquidRenderer.h"

#include <fstream>
#include <iomanip>

#include "CNCompactNSearch.h"
#include "SVD.h"

#include "oneapi/tbb.h"

LiquidRenderer::LiquidRenderer()
{
	m_vbos.resize(1);
	m_pointSpriteShader.load("shaders/particle.vert", "shaders/particle.frag");
	m_depthPassShader.load("shaders/depth-pass.vert", "shaders/depth-pass.frag");
	m_thickPassShader.load("shaders/thickness-pass.vert", "shaders/thickness-pass.frag");
	m_compositionShader.load("shaders/composition-pass.vert", "shaders/composition-pass.frag");

	ofVec3f screenQuadVertices[] =
	{
		ofVec3f(+1, +1, 0),
		ofVec3f(-1, +1, 0),
		ofVec3f(-1, -1, 0),
		ofVec3f(+1, -1, 0)
	};

	m_screenQuadVbo.setVertexData(screenQuadVertices, 4, GL_STATIC_DRAW);

}

void LiquidRenderer::setup(int maxNumParticles, int totalFrames, const std::string &root)
{
	m_maxNumParticles = maxNumParticles;
	m_totalFrames = totalFrames;

	//Memory allocation
	m_vertices.reset(new ofVec3f[m_maxNumParticles]);
	m_colors.reset(new ofFloatColor[m_maxNumParticles]);
	m_anisotropyMatrices.reset(new float[m_maxNumParticles * 9]);

	//Random color
	ofSeedRandom();
	ofFloatColor color;
	for (int i = 0; i < m_maxNumParticles; ++i)
	{
		color.r = 0.117f;
		color.g = 0.384f;
		color.b = 0.764f;
		color.a = 1.0f;
		m_colors[i] = color;
	}

	const int float3Size = sizeof(float) * 3;
	m_vbos[0].setVertexData(m_vertices.get(), m_maxNumParticles, GL_DYNAMIC_DRAW);
	m_vbos[0].setColorData(m_colors.get(), m_maxNumParticles, GL_DYNAMIC_DRAW);
	m_vbos[0].setAttributeData(4, &m_anisotropyMatrices.get()[0], 3, m_maxNumParticles, GL_DYNAMIC_DRAW, 3 * float3Size);
	m_vbos[0].setAttributeData(5, &m_anisotropyMatrices.get()[3], 3, m_maxNumParticles, GL_DYNAMIC_DRAW, 3 * float3Size);
	m_vbos[0].setAttributeData(6, &m_anisotropyMatrices.get()[6], 3, m_maxNumParticles, GL_DYNAMIC_DRAW, 3 * float3Size);

	m_currentFrame = 1;

	//Setup directory path
	m_rootPath = root;

	ofFboSettings settings;
	settings.width = ofGetWindowWidth();
	settings.height = ofGetWindowHeight();
	settings.colorFormats = { GL_R32F };
	settings.useDepth = true;
	settings.useStencil = false;
	settings.depthStencilAsTexture = true;
	m_fbo[0].allocate(settings);
	m_fbo[1].allocate(settings);
}

void LiquidRenderer::update()
{
	//Load the corresponding data
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

	readFluidPosition(m_currentFrame);
	readAnisotropyMatrices(m_currentFrame);

	//Move forward to the next frame
	m_vbos[0].updateVertexData(m_vertices.get(), m_numParticles);
	m_vbos[0].updateColorData(m_colors.get(), m_numParticles);
	m_vbos[0].updateAttributeData(4, &m_anisotropyMatrices.get()[0], m_numParticles);
	m_vbos[0].updateAttributeData(5, &m_anisotropyMatrices.get()[3], m_numParticles);
	m_vbos[0].updateAttributeData(6, &m_anisotropyMatrices.get()[6], m_numParticles);
}

void LiquidRenderer::draw(const glm::mat4 &viewMatrix, const glm::mat4 &projectMatrix,
	const glm::vec3 &lightPos, const glm::vec3 &lightColor, const glm::vec3 &cameraPos)
{
	//Draw particles
	int width = ofGetWindowWidth();
	int height = ofGetWindowHeight();

	ofEnablePointSprites();

	m_pointSpriteShader.begin();

	m_pointSpriteShader.setUniform3f("u_light.position", lightPos);
	m_pointSpriteShader.setUniform3f("u_light.lightColor", lightColor);


	m_pointSpriteShader.setUniformMatrix4f("u_viewMatrix", viewMatrix);
	m_pointSpriteShader.setUniformMatrix4f("u_projectMatrix", projectMatrix);
	m_pointSpriteShader.setUniformMatrix4f("u_invViewMatrix", glm::inverse(viewMatrix));
	m_pointSpriteShader.setUniformMatrix4f("u_invProjectMatrix", glm::inverse(projectMatrix));

	m_pointSpriteShader.setUniform1f("u_pointRadius", m_particleRadius);
	m_pointSpriteShader.setUniform1i("u_screenWidth", width);
	m_pointSpriteShader.setUniform1i("u_screenHeight", height);
	m_pointSpriteShader.setUniform1i("u_useAnisotropyKernel", m_useAnisotropyKernel ? 1 : 0);

	m_vbos[0].draw(GL_POINTS, 0, m_numParticles);

	m_pointSpriteShader.end();
}

void LiquidRenderer::drawSurface(const glm::mat4 &viewMatrix, const glm::mat4 &projectMatrix,
	const glm::vec3 &lightPos, const glm::vec3 &lightColor, const glm::vec3 &cameraPos, 
	ofTexture &backgroundTex, ofTexture &backgroundDepth, ofTexture &skyboxTex)
{
	int width = ofGetWindowWidth();
	int height = ofGetWindowHeight();

	//Depth pass
	{
		m_fbo[0].begin();

		ofEnableDepthTest();
		ofEnablePointSprites();
		ofDisableAlphaBlending();
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glClearDepth(1.0f);

		m_depthPassShader.begin();

		m_depthPassShader.setUniformMatrix4f("u_viewMatrix", viewMatrix);
		m_depthPassShader.setUniformMatrix4f("u_projectMatrix", projectMatrix);
		m_depthPassShader.setUniformMatrix4f("u_invViewMatrix", glm::inverse(viewMatrix));
		m_depthPassShader.setUniformMatrix4f("u_invProjectMatrix", glm::inverse(projectMatrix));

		m_depthPassShader.setUniform1f("u_pointRadius", m_particleRadius);
		m_depthPassShader.setUniform1i("u_screenWidth", width);
		m_depthPassShader.setUniform1i("u_screenHeight", height);
		m_depthPassShader.setUniform1i("u_useAnisotropyKernel", m_useAnisotropyKernel ? 1 : 0);

		m_vbos[0].draw(GL_POINTS, 0, m_numParticles);

		m_depthPassShader.end();
		m_fbo[0].end();
	}

	//Thickness pass
	{
		m_fbo[1].begin();
		ofEnableAlphaBlending();
		ofDisableDepthTest();
		ofEnablePointSprites();
		glClear(GL_COLOR_BUFFER_BIT);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_ONE, GL_ONE);

		m_thickPassShader.begin();

		m_thickPassShader.setUniformMatrix4f("u_viewMatrix", viewMatrix);
		m_thickPassShader.setUniformMatrix4f("u_projectMatrix", projectMatrix);
		m_thickPassShader.setUniform1f("u_pointRadius", m_particleRadius);
		m_thickPassShader.setUniform1i("u_screenWidth", width);
		m_thickPassShader.setUniform1i("u_screenHeight", height);

		m_vbos[0].draw(GL_POINTS, 0, m_numParticles);

		m_thickPassShader.end();

		ofDisableAlphaBlending();
		ofEnableDepthTest();
		glDepthMask(GL_TRUE);
		m_fbo[1].end();
	}

	//Depthe filtering
	ofTexture *colorTex = &m_fbo[0].getTexture();
	if (m_depthFilter != nullptr)
	{
		m_depthFilter->setParticleRadius(m_particleRadius);
		colorTex = &m_depthFilter->blurTexture(m_fbo[0].getTexture());
	}

	//Composition pass
	{
		m_compositionShader.begin();

		m_compositionShader.setUniform3f("u_light.position", lightPos);
		m_compositionShader.setUniform3f("u_light.lightColor", lightColor);
		m_compositionShader.setUniformMatrix4f("u_viewMatrix", viewMatrix);
		m_compositionShader.setUniformMatrix4f("u_projectMatrix", projectMatrix);
		m_compositionShader.setUniformMatrix4f("u_invViewMatrix", glm::inverse(viewMatrix));
		m_compositionShader.setUniformMatrix4f("u_invProjectMatrix", glm::inverse(projectMatrix));
		m_compositionShader.setUniform1i("u_screenWidth", width);
		m_compositionShader.setUniform1i("u_screenHeight", height);
		m_compositionShader.setUniform1f("u_reflectionConstant", 0.0f);
		m_compositionShader.setUniform1f("u_attennuationConstant", 0.99f);
		m_compositionShader.setUniform1i("u_transparentFluid", m_transparentFluid ? 1 : 0);
		m_compositionShader.setUniform1i("u_renderType", u_renderType);

		m_compositionShader.setUniformTexture("u_depthTex", *colorTex, 0);
		m_compositionShader.setUniformTexture("u_thicknessTex", m_fbo[1].getTexture(), 1);
		m_compositionShader.setUniformTexture("u_backgroundTex", backgroundTex, 2);
		m_compositionShader.setUniformTexture("u_backgroundDepthTex", backgroundDepth, 3);
		m_compositionShader.setUniformTexture("u_skyBoxTex", skyboxTex, 4);

		m_screenQuadVbo.draw(GL_TRIANGLE_FAN, 0, 4);

		m_compositionShader.end();

	}

}

bool LiquidRenderer::readFluidPosition(int frameId)
{
	std::stringstream ss;
	ss << "FluidFrame/frame." << setfill('0') << setw(4) << frameId << ".pos";

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

	//The number of particles
	int  numParticles;
	float particleRadius;
	memcpy(&numParticles, m_readBuffer.data(), sizeof(int));
	memcpy(&particleRadius, &m_readBuffer.data()[sizeof(int)], sizeof(float));
	m_numParticles = numParticles;
	m_particleRadius = particleRadius;
	assert(m_numParticles > 0);
	assert(m_particleRadius > 0);

	//Copy the vertices data
	memcpy(m_vertices.get(), &m_readBuffer.data()[sizeof(int) + sizeof(float)], 3 * numParticles * sizeof(float));

	return true;
}

bool LiquidRenderer::readAnisotropyMatrices(int frameId)
{
	if (false)
	{
		// Calculate the anisotropy kernels on the fly
		generateAniKernels();
	}
	else
	{
		// Just load the anisotropy kernels from precomputed files.
		std::stringstream ss;
		ss << "FluidFrame/frame." << setfill('0') << setw(4) << frameId << ".ani";

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

		int numParticles;
		memcpy(&numParticles, m_readBuffer.data(), sizeof(int));
		assert(numParticles == m_numParticles);

		size_t dataSize = 9 * numParticles * sizeof(float);
		assert(dataSize + sizeof(int) == fileSize);
		memcpy(m_anisotropyMatrices.get(), &m_readBuffer.data()[sizeof(int)], dataSize);
	}
}

#define AniGen_Lambda                 0.0
#define AniGen_NeighborCountThreshold 25
#define AniGen_Kr                     4

void LiquidRenderer::generateAniKernels()
{
	// Refs: https://github.com/ttnghia/SPlisHSPlasH/blob/master/SPlisHSPlasH/FluidModel.cpp

	static float aniKernelRadius = 8.0 * m_particleRadius;
	static float aniKernelRadiusInv = 1.0 / m_particleRadius;
	static float aniKernelRadiusSqr = aniKernelRadius * aniKernelRadius;
	const int numFluidParticles = (int)m_numParticles;
	static auto kernelW = [](float d, float aniKernelRadiusInv)
	{
		return 1.0 - pow(d * aniKernelRadiusInv, 3);
	};

	float supportRadius = 4.0f * m_particleRadius;
	std::unique_ptr<CompactNSearch::NeighborhoodSearch> aniNeighborhoodSearch = nullptr;

	aniNeighborhoodSearch.reset(new CompactNSearch::NeighborhoodSearch(supportRadius, false));
	aniNeighborhoodSearch->set_radius(aniKernelRadius);
	aniNeighborhoodSearch->add_point_set(&m_vertices[0][0], m_numParticles, false, true);
	aniNeighborhoodSearch->find_neighbors();

	auto vec3_mul_to_mat3 = [](const glm::vec3 &l, const glm::vec3 &r) -> glm::mat3
	{
		glm::mat3 ans;
		ans[0] = l * r.x;
		ans[1] = l * r.y;
		ans[2] = l * r.z;
		return ans;
	};

	tbb::parallel_for(0, numFluidParticles, [&](const int &i) -> void
	{
		const glm::vec3 &xi = m_vertices[i];

		glm::vec3 pposWM = xi;
		float sumW = 1.0;
		unsigned int numNeighbors = static_cast<unsigned int>(aniNeighborhoodSearch->point_set(0).n_neighbors(i));

		for (unsigned int j = 0; j < numNeighbors; j++)
		{
			const CompactNSearch::PointID &particleId = aniNeighborhoodSearch->point_set(0).neighbor(i, j);
			const glm::vec3 &xj = m_vertices[particleId.point_id];

			const glm::vec3 xij = xj - xi;
			const float d2 = glm::length2(xij);
			if (d2 < aniKernelRadiusSqr)
			{
				const float wij = kernelW(glm::sqrt(d2), aniKernelRadiusInv);
				sumW += wij;
				pposWM += wij * xj;
			}
		}

		assert(sumW > 0);
		pposWM /= sumW;

		////////////////////////////////////////////////////////////////////////////////
		// compute covariance matrix and anisotropy matrix
		unsigned int neighborCount = 0;
		//Matrix3r C = (xi - pposWM)*(xi - pposWM).transpose();
		glm::mat3 C = vec3_mul_to_mat3(xi - pposWM, xi - pposWM);
		sumW = 1.0;

		for (unsigned int j = 0; j < numNeighbors; j++)
		{
			const CompactNSearch::PointID &particleId = aniNeighborhoodSearch->point_set(0).neighbor(i, j);
			const glm::vec3 &xj = m_vertices[particleId.point_id];

			const glm::vec3 xij = xj - pposWM;
			const float d2 = glm::length2(xij);
			if (d2 < aniKernelRadiusSqr)
			{
				const float wij = kernelW(sqrt(d2), aniKernelRadiusInv);
				sumW += wij;

				//C += wij * xij * xij.transpose();
				C += wij * vec3_mul_to_mat3(xij, xij);

				++neighborCount;
			}
		}

		assert(sumW > 0);
		C /= sumW; // = covariance matrix

		////////////////////////////////////////////////////////////////////////////////
		// compute kernel matrix
		glm::mat3 U, S, V;
		// Note: glm is column-major
		SVDDecomposition::svd(
			C[0][0], C[1][0], C[2][0], C[0][1], C[1][1], C[2][1], C[0][2], C[1][2], C[2][2],
			U[0][0], U[1][0], U[2][0], U[0][1], U[1][1], U[2][1], U[0][2], U[1][2], U[2][2],
			S[0][0], S[1][0], S[2][0], S[0][1], S[1][1], S[2][1], S[0][2], S[1][2], S[2][2],
			V[0][0], V[1][0], V[2][0], V[0][1], V[1][1], V[2][1], V[0][2], V[1][2], V[2][2]);

		glm::vec3 sigmas = static_cast<float>(0.75f) * glm::vec3(1, 1, 1);

		if (neighborCount > AniGen_NeighborCountThreshold)
		{
			sigmas = glm::vec3(S[0][0], std::max(S[1][1], S[0][0] / AniGen_Kr), std::max(S[2][2], S[0][0] / AniGen_Kr));
			float ks = std::cbrt(1.0 / (sigmas[0] * sigmas[1] * sigmas[2]));          // scale so that det(covariance) == 1
			sigmas *= ks;
		}

		S = glm::mat3(0.0f);
		S[0][0] = sigmas[0];
		S[1][1] = sigmas[1];
		S[2][2] = sigmas[2];
		glm::mat3 kernelMatrix = U * S * glm::transpose(U);

		m_anisotropyMatrices[9 * i + 0] = kernelMatrix[0][0];
		m_anisotropyMatrices[9 * i + 1] = kernelMatrix[0][1];
		m_anisotropyMatrices[9 * i + 2] = kernelMatrix[0][2];

		m_anisotropyMatrices[9 * i + 3] = kernelMatrix[1][0];
		m_anisotropyMatrices[9 * i + 4] = kernelMatrix[1][1];
		m_anisotropyMatrices[9 * i + 5] = kernelMatrix[1][2];

		m_anisotropyMatrices[9 * i + 6] = kernelMatrix[2][0];
		m_anisotropyMatrices[9 * i + 7] = kernelMatrix[2][1];
		m_anisotropyMatrices[9 * i + 8] = kernelMatrix[2][2];
	});

}