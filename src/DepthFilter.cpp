#include "DepthFilter.h"

DepthFilter::DepthFilter() 
{
	ofVec3f screenQuadVertices[] =
	{
		ofVec3f(+1, +1, 0),
		ofVec3f(-1, +1, 0),
		ofVec3f(-1, -1, 0),
		ofVec3f(+1, -1, 0)
	};
	m_quadVbo.setVertexData(screenQuadVertices, 4, GL_STATIC_DRAW);
	m_blitShader.load("shaders/blit.vert", "shaders/blit.frag");
}

ofTexture& DepthFilter::blurTexture(ofTexture &target)
{
	int width = target.getWidth();
	int height = target.getHeight();	
	int frontbuffer = 0;
	int backbuffer = 1;
	
	//Allocation
	if (!m_fbos[frontbuffer].isAllocated() || width != m_fbos[frontbuffer].getWidth()
		|| height != m_fbos[frontbuffer].getHeight())
	{
		ofFboSettings settings;
		settings.width = width;
		settings.height = height;
		settings.colorFormats = { GL_R32F };
		settings.useDepth = true;
		settings.useStencil = false;
		settings.depthStencilAsTexture = true;
		m_fbos[frontbuffer].allocate(settings);
		m_fbos[backbuffer].allocate(settings);
	}
	
	//Blit the texture to front framebuffer
	{
		m_fbos[frontbuffer].begin();

		ofEnableDepthTest();
		ofBackground(0, 0, 0);
		glClearDepth(1.0f);
		ofDisableAlphaBlending();
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glDepthMask(GL_TRUE);

		m_fbos[frontbuffer].activateAllDrawBuffers();

		m_blitShader.begin();
		m_blitShader.setUniformTexture("u_targetTex", target, 0);

		m_quadVbo.draw(GL_TRIANGLE_FAN, 0, 4);

		m_blitShader.end();
		m_fbos[frontbuffer].end();
	}

	int horizontal = 1;
	//Blur the texture in an iterative manner
	for (int it = 0; it < m_numIter; ++it)
	{
		m_fbos[backbuffer].begin();

		ofEnableDepthTest();
		ofBackground(0, 0, 0);
		glClearDepth(1.0f);
		ofDisableAlphaBlending();
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glDepthMask(GL_TRUE);

		m_blurShader.begin();
		m_blurShader.setUniform1f("u_particleRadius", m_particleRadius);
		m_blurShader.setUniform1i("u_filterSize", m_filterSize);
		m_blurShader.setUniform1i("u_filterSize", m_filterSize);
		m_blurShader.setUniform1i("u_maxFilterSize", 20);
		m_blurShader.setUniform1i("u_screenWidth", width);
		m_blurShader.setUniform1i("u_screenHeight", height);
		m_blurShader.setUniform1i("horizontal", horizontal);
		m_blurShader.setUniformTexture("u_targetTex", m_fbos[frontbuffer].getTexture(), 0);
		setupUniforms();

		m_quadVbo.draw(GL_TRIANGLE_FAN, 0, 4);

		m_blurShader.end();
		m_fbos[backbuffer].end();
		horizontal = 1 - horizontal;
		std::swap(frontbuffer, backbuffer);
	}

	return m_fbos[frontbuffer].getTexture();
	
}

GaussianFilter::GaussianFilter()
{
	m_blurShader.load("shaders/filter-gaussian.vert", "shaders/filter-gaussian.frag");
}

void GaussianFilter::setupUniforms()
{
	//nothing
}

BiGaussianFilter::BiGaussianFilter() 
{
	m_blurShader.load("shaders/filter-bigaussian.vert", "shaders/filter-bigaussian.frag");
}

void BiGaussianFilter::setupUniforms()
{
	//nothing
}

CurvatureFilter::CurvatureFilter(glm::mat4 projectMatrix)
	: m_projectMatrix(projectMatrix)
{
	m_blurShader.load("shaders/filter-curvature-flow.vert", "shaders/filter-curvature-flow.frag");
}

void CurvatureFilter::setupUniforms()
{
	//nothing
	m_blurShader.setUniformMatrix4f("u_projectMatrix", m_projectMatrix);
}

NarrowFilter::NarrowFilter()
{
	m_blurShader.load("shaders/filter-narrow-range.vert", "shaders/filter-narrow-range.frag");
}

void NarrowFilter::setupUniforms()
{
	//nothing
}