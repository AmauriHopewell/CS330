///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
// Updated Sept. 2025 by Amauri Hopewell to add new shapes and modify view position
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

// declarations for global variables and defines
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// camera object used for viewing and interacting with
	// the 3D scene
	Camera* g_pCamera = nullptr;

	// these variables are used for mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// time between current frame and last frame
	float gDeltaTime = 0.0f; 
	float gLastFrame = 0.0f;

	// the following variable is false when orthographic projection
	// is off and true when it is on
	bool bOrthographicProjection = false;

	float gCameraSpeed = 2.5f; //to allow controlling via mouse scroll


	//Add new variables to allow more easily switching between positions
	int defaultCameraZoom = 68; //a good number for this scene found by experimenting
	glm::vec3 default3dCameraPosition = glm::vec3(-1.0f, 3.0f, 7.0f);
	glm::vec3 latestCameraPosition = default3dCameraPosition; //to allow returning to different views
	glm::vec3 latestCameraFront; 
	glm::vec3 defaultCameraFront = glm::normalize(glm::vec3(0.0f, -0.1f, -1.0f));
	glm::vec3 orthographicCameraPosition = glm::vec3(0.0f, 0.0f, 10.0f); // very head-on view
	glm::vec3 orthographicCameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Direct front view with floor plane hidden
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(
	ShaderManager *pShaderManager)
{
	// initialize the member variables
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();

	// default camera view parameters
	//OLD CAMERA PARAMS BELOW
	//g_pCamera->Position = glm::vec3(0.0f, 9.0f, 18.0f);
	//g_pCamera->Front = glm::vec3(0.0f, -0.8f, -3.0f);
	//g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	//g_pCamera->Zoom = 80;
	// 
	
	
	//Modified cmaera for a more head-on view
	g_pCamera->Position = default3dCameraPosition;  
	g_pCamera->Front = defaultCameraFront; 
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = defaultCameraZoom;  // FOV zoomed in even more
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
	// free up allocated memory
	m_pShaderManager = NULL;
	m_pWindow = NULL;
	if (NULL != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	// try to create the displayed OpenGL window
	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	// enable blending for supporting tranparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Addition for scroll callback
	glfwSetScrollCallback(window, &ViewManager::Scroll_Callback);


	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback); //to allow panning with the mouse


	// tell GLFW to capture all mouse events 
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 


	m_pWindow = window;

	return(window);
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	//std::cout << "aa11" << std::endl; //debug code

	// when the first mouse move event is received, this needs to be recorded so that
	// all subsequent mouse moves can correctly calculate the X position offset and Y
	// position offset for proper operation
	if (gFirstMouse)
	{
		gLastX = xMousePos;
		gLastY = yMousePos;
		gFirstMouse = false;
	}

	// calculate the X offset and Y offset values for moving the 3D camera accordingly
	float xOffset = xMousePos - gLastX;
	float yOffset = gLastY - yMousePos; // reversed since y-coordinates go from bottom to top

	// set the current positions into the last position variables
	gLastX = xMousePos;
	gLastY = yMousePos;

	// move the 3D camera according to the calculated offsets
	g_pCamera->ProcessMouseMovement(xOffset, yOffset);
	///std::cout << "aaa" << std::endl; //debug code

}

/***********************************************************
 *  Mouse_Scroll_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is scrolled within the active GLFW display window.
 ***********************************************************/
void ViewManager::Scroll_Callback(GLFWwindow* window, double xoffset, double yoffset)
{
	float sensitivity = 0.5f; //to choose how much it adjusts by. Picked through testing different values
	float speedUpperBound = 20.0;  //to avoid getting uncontrollably fast
	float speedLowerBound = 0.1; //to avoid negative numbers
	//printf("the speed was %f", cameraSpeed); //line dor debugging
	// Adjust camera speed based on scroll (yoffset >0 (scrolling up) causes faster zoom <0 (slowing down) causes slower zoom)

	gCameraSpeed += (float)yoffset * 0.5f;
	//printf("the speed is %f", cameraSpeed);//line for debugging

	if (gCameraSpeed < speedLowerBound)
	{
		gCameraSpeed = speedLowerBound;
	}  // Min speed clamp
	if (gCameraSpeed > speedUpperBound)
	{
		gCameraSpeed = speedUpperBound;
	} // Max speed clamp


	//std::cout << "bbb" << std::endl; //debug code

}


/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
	// close the window if the escape key has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}
	

	// if the camera object is null, then exit this method
	if (NULL == g_pCamera)
	{
		return;
	}

	// process camera zooming in and out
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(FORWARD, gCameraSpeed * gDeltaTime); // modify to change speed according to mouse scroll
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(BACKWARD, gCameraSpeed * gDeltaTime); // modify to change speed according to mouse scroll
	}

	// process camera panning left and right
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(LEFT, gCameraSpeed * gDeltaTime); // modify to change speed according to mouse scroll
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(RIGHT, gCameraSpeed * gDeltaTime); // modify to change speed according to mouse scroll
	}


	//ADDITION: process camera moving verticalle
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(UP, gCameraSpeed * gDeltaTime); // modify to change speed according to mouse scroll
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(DOWN, gCameraSpeed * gDeltaTime); // modify to change speed according to mouse scroll
	}

	//printf("the speed is %f", cameraSpeed); //debug code to verify scroll_callback performance

	//ADDITION: switch between orthographic and projection views
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(UP, gCameraSpeed * gDeltaTime); // modify to change speed according to mouse scroll
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(DOWN, gCameraSpeed * gDeltaTime); // modify to change speed according to mouse scroll
	}


	// ADDITION: Toggle projection and orthographic mode
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
	{
		//save view if not already in orthographic view
		if (bOrthographicProjection == false)
		{
			std::cout << "aa\n";
			latestCameraPosition = g_pCamera->Position; 
			latestCameraFront = g_pCamera->Front;
		}
		bOrthographicProjection = true;
		// Reset camera to look directly at object
		g_pCamera->Front = orthographicCameraFront;  
		g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
		g_pCamera->Position = orthographicCameraPosition;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
	{
		if (bOrthographicProjection == true)
		{
			g_pCamera->Position = latestCameraPosition; //restore position if only pressing P once
														//press P twice to restore to default projection view
		}
		bOrthographicProjection = false;
		// Restore default perspective camera 
		g_pCamera->Position = default3dCameraPosition;
		g_pCamera->Front = defaultCameraFront;
		g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
		g_pCamera->Zoom = defaultCameraZoom;
	}

}




/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// per-frame timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// process any keyboard events that may be waiting in the 
	// event queue
	ProcessKeyboardEvents();

	// get the current view matrix from the camera
	view = g_pCamera->GetViewMatrix();

	// define the current projection matrix
	projection = glm::perspective(glm::radians(g_pCamera->Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

	// if the shader manager object is valid
	if (NULL != m_pShaderManager)
	{
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ViewName, view);
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		// set the view position of the camera into the shader for proper rendering
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}