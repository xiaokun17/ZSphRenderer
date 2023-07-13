#include "ofApp.h"
#include <chrono>
#include <fstream>

//--------------------------------------------------------------
void ofApp::setup()
{
	m_skyRenderer.reset(new SkyRenderer());
	m_liquidRenderer.reset(new LiquidRenderer());
	m_solidRenderer.reset(new SolidRenderer());
	m_floorRenderer.reset(new MeshRenderer());

	ofDisableArbTex();

	// background
	m_floorRenderer->setup("F:\\openframeworks\\apps\\myApps\\ZSphRenderer\\src\\data\\Floor\\floor2.obj");
	m_skyRenderer->setup("Sky/sky2/");

	// view/project matrix
	float width = ofGetWindowWidth();
	float height = ofGetWindowHeight();
	float aspect = width / height;
	// m_viewMatrix = glm::lookAt(m_cameraPos, m_focusPos, glm::vec3(0, 1, 0));
	m_projectMatrix = glm::perspective(45.0f, aspect, 0.1f, 1000.0f);


	ofFboSettings settings;
	settings.width = ofGetWindowWidth();
	settings.height = ofGetWindowHeight();
	settings.colorFormats = { GL_RGBA32F };
	settings.useDepth = true;
	settings.useStencil = false;
	settings.depthStencilAsTexture = true;
	m_backgroundFbo.allocate(settings);

	// gui setting
	m_gui.setup();
	m_openFile.addListener(this, &ofApp::openFileDialog);
	m_checkSkyBox.addListener(this, &ofApp::checkSkyBox);
	m_start.addListener(this, &ofApp::setStart);
	m_reset.addListener(this, &ofApp::setReset);
	m_drawParticles.addListener(this, &ofApp::setDrawParticles);
	m_useAnisotropyKenel.addListener(this, &ofApp::setUseAnisotropyKernel);
	m_gaussian.addListener(this, &ofApp::setGaussianFunction);
	m_biGaussian.addListener(this, &ofApp::setBiGaussianFunction);
	m_curvature.addListener(this, &ofApp::setCurvatureFunction);
	m_narrow.addListener(this, &ofApp::setNarrowFunction);


	m_gui.add(m_openFile.setup("Open Data"));
	m_gui.add(m_start.setup("Start"));
	m_gui.add(m_reset.setup("Reset"));
	m_gui.add(m_drawParticles.setup("Draw particles"));
	m_gui.add(m_gaussian.setup("Gaussian"));
	m_gui.add(m_biGaussian.setup("BiGaussian"));
	m_gui.add(m_curvature.setup("Curvature"));
	m_gui.add(m_narrow.setup("Narrow"));
	m_gui.add(m_useAnisotropyKenel.setup("Use anisotropy kernel"));
	m_gui.add(m_checkSkyBox.setup("Change Sky Box"));
	m_gui.add(m_iterSlider.setup("Iterations", m_config.m_numIteration, 0, 150));
	m_gui.add(m_filterSlider.setup("FilterSize", m_config.m_filterSize, 0, 50));

	string path = "F:\\openframeworks\\apps\\myApps\\ZSphRenderer\\data\\FluidSim2\\viz_info.txt";
	load(path);
	m_loaded = true;
}

//--------------------------------------------------------------
void ofApp::load(std::string &path)
{
	std::ifstream file;
	file.open(path, std::ios::in);

	if (!file.is_open())
	{
		std::cerr << "Failed to load the file: " << path << std::endl;
		return;
	}

	std::string line;
	int numSolids = 0;
	int maxNumParticles;
	int totalFrames;
	int solidFrames = 0;
	glm::vec3 cameraPos;
	glm::vec3 focusPos;

	while (std::getline(file, line))
	{
		if (line.empty())
			continue;

		std::stringstream ss;
		ss << line;
		std::string header;
		ss >> header;

		if (header == "max_fluid_particles")
		{
			ss >> maxNumParticles;
		}
		else if (header == "total_frames")
		{
			ss >> totalFrames;
		}
		else if (header == "camera_position")
		{
			ss >> cameraPos.x;
			ss >> cameraPos.y;
			ss >> cameraPos.z;
		}
		else if (header == "camera_focus")
		{
			ss >> focusPos.x;
			ss >> focusPos.y;
			ss >> focusPos.z;
		}
		else if (header == "light_position")
		{
			ss >> m_lightPos.x;
			ss >> m_lightPos.y;
			ss >> m_lightPos.z;
		}
		else if (header == "num_solids")
		{
			ss >> numSolids;
		}
		else if (header == "solid_frames")
		{
			ss >> solidFrames;
		}
		else if (header == "solids_color")
		{
			glm::vec3 color = glm::vec3(0, 0, 0);
			for (int i = 0; i < numSolids; i++)
			{
				ss >> color.x;
				ss >> color.y;
				ss >> color.z;
				m_solidRenderer->pushColor(color);
			}
		}
	}

	file.close();

	assert(maxNumParticles > 0);
	assert(totalFrames > 0);

	//Setup directory path
	std::string rootPath;
	size_t last_slash_idx = path.rfind('\\');
	if (last_slash_idx == std::string::npos)
	{
		last_slash_idx = path.rfind('/');
	}
	if (last_slash_idx != std::string::npos)
	{
		rootPath = path.substr(0, last_slash_idx + 1);
	}

	m_liquidRenderer->setup(maxNumParticles, totalFrames, rootPath);
	m_solidRenderer->setup(numSolids, solidFrames, rootPath);

	// m_cameraPos = cameraPos;
	m_focusPos = focusPos;
	m_viewMatrix = glm::lookAt(m_cameraPos, m_cameraPos + cameraFront, cameraUp);

	// Depth filter

	m_depthFilter.reset(new GaussianFilter());
	m_depthFilter->setFilterSize(m_filterSlider);

	//m_depthFilter.reset(new BiGaussianFilter());
	//m_depthFilter->setFilterSize(5);

	//m_depthFilter.reset(new CurvatureFilter(m_projectMatrix));
	//m_depthFilter->setFilterSize(5);

	//m_depthFilter.reset(new NarrowFilter());
	//m_depthFilter->setFilterSize(5);
}

//--------------------------------------------------------------
void ofApp::update()
{
	m_floorRenderer->update();
	m_skyRenderer->update();
	static chrono::steady_clock::time_point time_start;
	static chrono::steady_clock::time_point time_end;
	if (m_liquidRenderer->GetCurrentFrame() == 1)
	{
		time_start = chrono::steady_clock::now();
	}
	else if (m_liquidRenderer->GetCurrentFrame() == 30)
	{
		time_end = chrono::steady_clock::now();
		chrono::duration<double> time_used = chrono::duration_cast<chrono::duration<double>>(time_end - time_start);
		std::cout << "Printing took "
			<< time_used.count()
			<< "s.\n";
	}

	if (m_loaded)
	{
		m_liquidRenderer->update();
		m_solidRenderer->update();
	}

	std::stringstream strm;
	strm << " #particles: " << (m_liquidRenderer->getNumParticles() / 1000) << "k " << "fps: " << ofGetFrameRate();
	ofSetWindowTitle(strm.str());
}

//--------------------------------------------------------------
void ofApp::draw()
{
	if (!m_loaded)
	{
		ofBackground(0, 0, 0);
		ofEnableDepthTest();

		m_solidRenderer->draw(m_viewMatrix, m_projectMatrix, m_lightPos, m_lightColor, m_cameraPos);
		m_floorRenderer->draw(m_viewMatrix, m_projectMatrix, m_lightPos, m_lightColor, m_cameraPos);
		m_skyRenderer->draw(m_viewMatrix, m_projectMatrix, m_lightPos, m_lightColor, m_cameraPos);
	}
	else
	{
		switch (curBlur)
		{
		case 0:
			m_depthFilter.reset(new GaussianFilter());
			m_depthFilter->setFilterSize(m_filterSlider);
			break;
		case 1:
			m_depthFilter.reset(new BiGaussianFilter());
			m_depthFilter->setFilterSize(m_filterSlider);
			break;
		case 2:
			m_depthFilter.reset(new CurvatureFilter(m_projectMatrix));
			m_depthFilter->setFilterSize(m_filterSlider);
			break;
		case 3:
			m_depthFilter.reset(new NarrowFilter());
			m_depthFilter->setFilterSize(m_filterSlider);
			break;
		default:
			break;
		}

		m_liquidRenderer->setDraw(m_config.m_start);
		m_solidRenderer->setDraw(m_config.m_start);

		if (m_config.m_drawParticle)
		{
			//Draw fluid particles
			ofBackground(0, 0, 0);
			ofEnableDepthTest();

			m_solidRenderer->draw(m_viewMatrix, m_projectMatrix, m_lightPos, m_lightColor, m_cameraPos);
			m_floorRenderer->draw(m_viewMatrix, m_projectMatrix, m_lightPos, m_lightColor, m_cameraPos);
			m_skyRenderer->draw(m_viewMatrix, m_projectMatrix, m_lightPos, m_lightColor, m_cameraPos);
			m_liquidRenderer->draw(m_viewMatrix, m_projectMatrix, m_lightPos, m_lightColor, m_cameraPos);

		}
		else
		{
			//Draw fluid surface

			//Background rendering
			m_backgroundFbo.begin();
			ofBackground(0, 0, 0);
			ofEnableDepthTest();

			
			m_skyRenderer->draw(m_viewMatrix, m_projectMatrix, m_lightPos, m_lightColor, m_cameraPos);
			m_floorRenderer->draw(m_viewMatrix, m_projectMatrix, m_lightPos, m_lightColor, m_cameraPos);
			m_solidRenderer->draw(m_viewMatrix, m_projectMatrix, m_lightPos, m_lightColor, m_cameraPos);

			m_backgroundFbo.end();

			if (m_depthFilter != nullptr)
			{
				m_depthFilter->setIterNum(m_iterSlider);
				m_depthFilter->setFilterSize(m_filterSlider);
			}

			//Liquid mesh rendering
			m_liquidRenderer->setDepthFilter(m_depthFilter.get());
			m_liquidRenderer->drawSurface(m_viewMatrix, m_projectMatrix, m_lightPos, m_lightColor,
				m_cameraPos, m_backgroundFbo.getTexture(), m_backgroundFbo.getDepthTexture(), m_skyRenderer->getCubemap());
		}
	}


	//Gui rendering
	ofDisableDepthTest();
	m_gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	float cameraSpeed = 0.05f;
	if (key == 'w') 
	{
		m_cameraPos += cameraSpeed * cameraFront;
		m_viewMatrix = glm::lookAt(m_cameraPos, m_cameraPos + cameraFront, cameraUp);
	}
	else if (key == 's') 
	{
		m_cameraPos -= cameraSpeed * cameraFront;
		m_viewMatrix = glm::lookAt(m_cameraPos, m_cameraPos + cameraFront, cameraUp);
	}
	else if (key == 'a') 
	{
		m_cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		m_viewMatrix = glm::lookAt(m_cameraPos, m_cameraPos + cameraFront, cameraUp);
	}
	else if (key == 'd') 
	{
		m_cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		m_viewMatrix = glm::lookAt(m_cameraPos, m_cameraPos + cameraFront, cameraUp);
	}
	else if (key == 'z')
	{
		m_liquidRenderer->setRenderType(RenderTye::Common);
	}
	else if (key == 'x')
	{
		m_liquidRenderer->setRenderType(RenderTye::Thickness);
	}
	else if (key == 'c')
	{
		m_liquidRenderer->setRenderType(RenderTye::Depth);
	}
	else if (key == 'v')
	{
		m_liquidRenderer->setRenderType(RenderTye::Normal);
	}
	else if (key == 'b')
	{
		m_liquidRenderer->setTransparentFluid();
	}
	else if (key == 'n')
	{
		m_liquidRenderer->setTransparentFluid();
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{
	
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
	static bool first = true;
	if (first)
	{
		m_lastX = x;
		m_lastY = y;
		first = false;
	}

	float xoffset = x - m_lastX;
	float yoffset = m_lastY - y; // reversed since y-coordinates go from bottom to top
	m_lastX = x;
	m_lastY = y;

	float sensitivity = 0.1f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = -cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = -sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

	m_viewMatrix = glm::lookAt(m_cameraPos, m_cameraPos + cameraFront, cameraUp);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
	if (button == 0)
	{
		m_leftMouseButtonPressed = true;
		m_lastX = x;
		m_lastY = y;
	}

	if (button == 2)
	{
		m_rightMouseButtonPressed = true;
		m_lastX = x;
		m_lastY = y;
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
	if (button == 0)
	{
		m_leftMouseButtonPressed = false;
	}

	if (button == 2)
	{
		m_rightMouseButtonPressed = false;
	}
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{ 

}

void ofApp::openFileDialog()
{
	ofFileDialogResult openFileResult = ofSystemLoadDialog("Select a fluid data directory", false, "data/");

	//Check if the user opened a file
	if (openFileResult.bSuccess) 
	{
		ofFile file(openFileResult.getPath());
		if (file.exists())
		{
			file.close();
			load(openFileResult.getPath());
			m_loaded = true;
		}
	}
}

void ofApp::checkSkyBox()
{
	m_skyRenderer->setup("Sky/sky4/");
	m_floorRenderer->setup("C:\\Users\\xuyua\\Desktop\\openframeworks\\apps\\myApps\\ZSphRenderer\\src\\data/Floor/floor2.obj");
}


void ofApp::setDrawParticles()
{
	m_config.m_drawParticle = !m_config.m_drawParticle;
}

void ofApp::setStart()
{
	m_config.m_start = !m_config.m_start;
}

void ofApp::setReset()
{
	m_liquidRenderer->setReset(true);
	m_solidRenderer->setReset(true);
}

void ofApp::setUseAnisotropyKernel()
{
	m_config.m_useAnisotropyKernel = !m_config.m_useAnisotropyKernel;
	m_liquidRenderer->setUseAnisotropyKernel(m_config.m_useAnisotropyKernel);
}

void ofApp::setGaussianFunction()
{
	curBlur = 0;
}

void ofApp::setBiGaussianFunction()
{
	curBlur = 1;
}

void ofApp::setCurvatureFunction()
{
	curBlur = 2;
}

void ofApp::setNarrowFunction()
{
	curBlur = 3;
}