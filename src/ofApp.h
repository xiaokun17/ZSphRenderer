#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include "LiquidRenderer.h"
#include "SolidRenderer.h"
#include "SkyRenderer.h"
#include "MeshRenderer.h"
#include "DepthFilter.h"

class ofApp : public ofBaseApp
{
public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

private:
	void openFileDialog();
	void checkSkyBox();
	void setDrawParticles();
	void setStart();
	void setReset();
	void setUseAnisotropyKernel();
	void setGaussianFunction();
	void setBiGaussianFunction();
	void setCurvatureFunction();
	void setNarrowFunction();
	int curBlur = 1;

private:
	void load(std::string &path);
	 
	bool m_loaded = false;
	glm::mat4 m_viewMatrix;
	glm::mat4 m_projectMatrix;

	glm::vec3 m_lightPos = glm::vec3(-3, 8, 7);
	glm::vec3 m_lightColor = glm::vec3(1, 1, 1);
	glm::vec3 m_cameraPos = glm::vec3(-0.6921f, 1.2116f, -4.0221f);
	glm::vec3 m_focusPos;
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	GLfloat yaw = -90.0f;
	GLfloat pitch = 0.0f;
	
	std::unique_ptr<LiquidRenderer> m_liquidRenderer = nullptr;
	std::unique_ptr<SolidRenderer> m_solidRenderer = nullptr;
	std::unique_ptr<SkyRenderer> m_skyRenderer = nullptr;
	std::unique_ptr<MeshRenderer> m_floorRenderer = nullptr;
	std::unique_ptr<DepthFilter> m_depthFilter = nullptr;

	ofFbo m_backgroundFbo;

	bool m_leftMouseButtonPressed = false;
	bool m_rightMouseButtonPressed = false;
	int m_lastX, m_lastY;

	//GUI
	ofxPanel				m_gui;
	ofxIntSlider			m_iterSlider;
	ofxIntSlider			m_filterSlider;
	ofxButton				m_openFile;
	ofxButton				m_checkSkyBox;
	ofxButton				m_start;
	ofxButton				m_reset;
	ofxButton				m_drawParticles;
	ofxButton				m_useAnisotropyKenel;
	ofxButton				m_gaussian;
	ofxButton				m_biGaussian;
	ofxButton				m_curvature;
	ofxButton				m_narrow;

	struct Config
	{
		bool m_start 					= false;
		bool m_reset 					= false;
		bool m_drawParticle				= false;
		bool m_useAnisotropyKernel		= false;
		int  m_numIteration				= 2;
		int  m_filterSize				= 4;
	};
	
	Config			m_config;
};