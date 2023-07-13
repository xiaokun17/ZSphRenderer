#pragma once

#include "ofMain.h"

class DepthFilter
{
public:

	DepthFilter();
	virtual ~DepthFilter() = default;

	virtual ofTexture& blurTexture(ofTexture &target);

	void setIterNum(int iter) { m_numIter = iter; }
	void setFilterSize(float size) { m_filterSize = size; }
	void setMaxFilterSize(float size) { m_maxFilterSize = size; }
	void setParticleRadius(float radius) { m_particleRadius = radius; }

protected:
	virtual void setupUniforms() = 0;

protected:
	ofShader  m_blitShader;
	ofShader  m_blurShader;
	ofVbo	  m_quadVbo;
	ofFbo	  m_fbos[2];
	int		  m_numIter = 1;

	float m_particleRadius;
	float m_filterSize;
	float m_maxFilterSize;
};

class GaussianFilter final : public DepthFilter
{
public:

	GaussianFilter();

protected:
	virtual void setupUniforms() override;

};

class BiGaussianFilter final : public DepthFilter
{
public:

	BiGaussianFilter();

protected:
	virtual void setupUniforms() override;

};

class CurvatureFilter final : public DepthFilter
{
public:
	
	CurvatureFilter(glm::mat4 projectMatrix);

protected:
	virtual void setupUniforms() override;

private:
	glm::mat4 m_projectMatrix;
};

class NarrowFilter final : public DepthFilter
{
public:

	NarrowFilter();

protected:
	virtual void setupUniforms() override;

};