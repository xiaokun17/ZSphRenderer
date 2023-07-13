#pragma once

#include "ofMain.h"
#include "ofShader.h"

#include "SphRenderer.h"
#include "DepthFilter.h"

enum RenderTye
{
	Common = 0,
	Depth,
	Thickness,
	Normal
};

class LiquidRenderer final : public SphRenderer
{
public:

	LiquidRenderer();

	void setup(int maxNumParticles, int totalFrames, const std::string &root);

	virtual void update() override;

	virtual void draw(const glm::mat4 &viewMatrix, const glm::mat4 &projectMatrix,
		const glm::vec3 &lightPos, const glm::vec3 &lightColor, const glm::vec3 &cameraPos) override;

	void drawSurface(const glm::mat4 &viewMatrix, const glm::mat4 &projectMatrix,
		const glm::vec3 &lightPos, const glm::vec3 &lightColor, const glm::vec3 &cameraPos,
		ofTexture &backgroundTex, ofTexture &backgroundDepth, ofTexture &skyboxTex);

	int getNumParticles() const { return m_numParticles; }
	float getParticleRadius() const { return m_particleRadius; }
	void setDepthFilter(DepthFilter *filter) { m_depthFilter = filter; }
	void setUseAnisotropyKernel(bool use) { m_useAnisotropyKernel = use; }
	void setDraw(bool draw) { m_draw = draw; }
	void setReset(bool reset) { m_reset = reset; }
	void setRenderType(RenderTye type) { u_renderType = type; }
	void setTransparentFluid() { m_transparentFluid = !m_transparentFluid; }
	int GetCurrentFrame() { return m_currentFrame; }
private:
	bool readFluidPosition(int frameId);
	bool readAnisotropyMatrices(int frameId);

	void generateAniKernels();

private:
	bool							m_useAnisotropyKernel = false;
	bool							m_calcAniOnTheFly = true;
	bool							m_draw = false;
	bool							m_reset = false;
	int								m_maxNumParticles;
	int								m_numParticles;
	bool							m_transparentFluid = false;
	RenderTye						u_renderType = RenderTye::Common;
	float							m_particleRadius;

	std::unique_ptr<ofVec3f[]>      m_vertices;
	std::unique_ptr<ofFloatColor[]> m_colors;
	std::unique_ptr<float[]>		m_anisotropyMatrices;

	ofShader						m_pointSpriteShader;
	ofShader						m_depthPassShader;
	ofShader						m_thickPassShader;
	ofShader						m_compositionShader;

	ofFbo							m_fbo[2];

	ofVbo							m_screenQuadVbo;

	DepthFilter*					m_depthFilter = nullptr;

};