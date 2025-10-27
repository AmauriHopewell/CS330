///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
// Updated Sept. 28 2025 by Amauri Hopewell to create 3d shape of clock as milestone
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseTwoTexturesName = "bUseTwoTextures"; //added to allow multiple texture shading
	const char* g_UseLightingName = "bUseLighting";

	//add these to allow scaling texture to the object
	const char* g_ObjectPositionName = "objectPosition";
	const char* g_ObjectScaleName = "objectScale";

	//set the torus major radius (inside the torus, set while declaring scale) 
	// and outer radius (rim thickness, set here, for use in PrepareScene)
	float torusMinorRadius = .05;

	
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
	m_objectMaterials.clear();
}


/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}


/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	// matrix math for calculating the final model matrix
	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		// pass the model matrix into the shader
		m_pShaderManager->setMat4Value(g_ModelName, modelView);

		//new code to allow fitting to object
		m_pShaderManager->setVec3Value("objectPosition", positionXYZ);
		m_pShaderManager->setVec3Value("objectScale", scaleXYZ);

	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		// pass the color values into the shader
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}



//Add this in to allow shaders that reflect light
/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		// find the defined material that matches the tag
		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			// pass the material properties into the shader
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/


//add materials for textures based on 6-2 and sample code
void SceneManager::DefineObjectMaterials()
{
	/*** STUDENTS - add the code BELOW for defining object materials. ***/
	/*** There is no limit to the number of object materials that can ***/
	/*** be defined. Refer to the code in the OpenGL Sample for help  ***/
	OBJECT_MATERIAL goldMaterial;
	goldMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.1f);
	goldMaterial.ambientStrength = 0.8f;
	goldMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.2f);
	goldMaterial.specularColor = glm::vec3(0.6f, 0.5f, 0.4f);
	goldMaterial.shininess = 22.0;
	goldMaterial.tag = "gold";
	m_objectMaterials.push_back(goldMaterial);
	OBJECT_MATERIAL cementMaterial;
	cementMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
	cementMaterial.ambientStrength = 0.2f;
	cementMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	cementMaterial.specularColor = glm::vec3(0.4f, 0.4f, 0.4f);
	cementMaterial.shininess = 0.5;
	cementMaterial.tag = "cement";
	m_objectMaterials.push_back(cementMaterial);
	OBJECT_MATERIAL woodMaterial;
	woodMaterial.ambientColor = glm::vec3(0.4f, 0.3f, 0.1f);
	woodMaterial.ambientStrength = 0.2f;
	woodMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f);
	woodMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	woodMaterial.shininess = 0.3;
	woodMaterial.tag = "wood";
	m_objectMaterials.push_back(woodMaterial);
	OBJECT_MATERIAL tileMaterial;
	tileMaterial.ambientColor = glm::vec3(0.2f, 0.3f, 0.4f);
	tileMaterial.ambientStrength = 0.3f;
	tileMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f);
	tileMaterial.specularColor = glm::vec3(0.4f, 0.5f, 0.6f);
	tileMaterial.shininess = 25.0;
	tileMaterial.tag = "tile";
	m_objectMaterials.push_back(tileMaterial);
	OBJECT_MATERIAL glassMaterial;
	glassMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
	glassMaterial.ambientStrength = 0.3f;
	glassMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
	glassMaterial.specularColor = glm::vec3(0.6f, 0.6f, 0.6f);
	glassMaterial.shininess = 85.0;
	glassMaterial.tag = "glass";
	m_objectMaterials.push_back(glassMaterial);
	OBJECT_MATERIAL clayMaterial;
	clayMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.3f);
	clayMaterial.ambientStrength = 0.3f;
	clayMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.5f);
	clayMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.4f);
	clayMaterial.shininess = 0.5;
	clayMaterial.tag = "clay";
	m_objectMaterials.push_back(clayMaterial);


	OBJECT_MATERIAL pinkMaterial;
	pinkMaterial.ambientColor = glm::vec3(0.6f, 0.3f, 0.5f);   // Boosted pink ambient
	pinkMaterial.ambientStrength = 0.3f / 4.0f;  // Divided by 4 to counter per-light addition
	pinkMaterial.diffuseColor = glm::vec3(0.9f, 0.5f, 0.7f) / 2.0f;   // Boosted and divided by 2 bright lights
	pinkMaterial.specularColor = glm::vec3(1.0f, 0.8f, 0.9f) / 2.0f;  // Divided by 2 bright lights
	pinkMaterial.shininess = 16.0f;  // A bit less than the gold texture
	pinkMaterial.tag = "pink";
	m_objectMaterials.push_back(pinkMaterial);

	OBJECT_MATERIAL blueMaterial;
	blueMaterial.ambientColor = glm::vec3(0.15f, 0.15f, 0.5f);
	blueMaterial.ambientStrength = 0.4f / 4.0f; //Divided by 4 since 4 lights
	blueMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.9f) / 2.0f; //Divided by 2 since 2 bright lights
	blueMaterial.specularColor = glm::vec3(0.7f, 0.7f, 1.0f) / 2.0f; //Divided by 2 since 2 bright lights
	blueMaterial.shininess = 32.0f;  // Higher shine
	blueMaterial.tag = "blue";
	m_objectMaterials.push_back(blueMaterial);


	OBJECT_MATERIAL brownMaterial;
	brownMaterial.ambientColor = glm::vec3(0.4f, 0.2f, 0.15f);
	brownMaterial.ambientStrength = 0.2f / 4.0f; //Divided by 4 since 4 lights
	brownMaterial.diffuseColor = glm::vec3(0.6f, 0.4f, 0.3f) / 2.0f; //Divided by 2 since 2 bright lights
	brownMaterial.specularColor = glm::vec3(0.7f, 0.5f, 0.4f) / 2.0f;//Divided by 2 since 2 bright lights
	brownMaterial.shininess = 8.0f;   // Low shine
	brownMaterial.tag = "brown";
	m_objectMaterials.push_back(brownMaterial);

	OBJECT_MATERIAL redMaterial;
	redMaterial.ambientColor = glm::vec3(0.5f, 0.15f, 0.15f);
	redMaterial.ambientStrength = 0.4f / 4.0f; //Divided by 4 since 4 lights
	redMaterial.diffuseColor = glm::vec3(0.9f, 0.3f, 0.3f) / 2.0f; //Divided by 2 since 2 bright lights
	redMaterial.specularColor = glm::vec3(1.0f, 0.6f, 0.6f) / 2.0f; //Divided by 2 since 2 bright lights
	redMaterial.shininess = 32.0f;  // Higher shine
	redMaterial.tag = "red";
	m_objectMaterials.push_back(redMaterial);


}

//Code based on examples lighting and 6-2 assignment experimenting
/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.  There are up to 4 light sources.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	// this line of code is NEEDED for telling the shaders to render 
	// the 3D scene with custom lighting, if no light sources have
	// been added then the display window will be black - to use the 
	// default OpenGL lighting then comment out the following line
	//m_pShaderManager->setBoolValue(g_UseLightingName, true);

	/*** STUDENTS - add the code BELOW for setting up light sources ***/
	/*** Up to four light sources can be defined. Refer to the code ***/
	/*** in the OpenGL Sample for help                              ***/
	/*
	m_pShaderManager->setVec3Value("lightSources[0].position", -3.0f, 4.0f, 6.0f);
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.70f, 0.1f, 0.1f);
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 0.0f, 0.0f, 0.0);
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.05f);
	m_pShaderManager->setVec3Value("lightSources[1].position", 3.0f, 4.0f, 6.0f);
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.4f, 0.4f, 0.4f);
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.4f, 0.0f, 0.0f);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.05f);
	m_pShaderManager->setVec3Value("lightSources[2].position", 0.0f, 3.0f, 20.0f);
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", 0.51f, 0.01f, 0.01f);
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", 0.3f, 0.3f, 0.3f);
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", 0.6f, 0.3f, 0.3f);
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 12.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.5f);
	*/
	m_pShaderManager->setBoolValue("bUseLighting", true);


	// Common settings: focalStrength (specular exponent) unchanged, but lower specularIntensity to dim highlights
	float commonFocalStrength = 32.0f;  // Matches shiniest material shininess
	float commonSpecularIntensity = 0.2f;  // Lowered from 0.3f to reduce base overexposure

	// Light 0: Blue directional with lowered position 
	m_pShaderManager->setVec3Value("lightSources[0].position", glm::vec3(3.0f, 10.0f, 4.0f));
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", glm::vec3(0.0f, 0.0f, 0.0f));  // Zero because additive ambient was making everything white
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", glm::vec3(0.5f, 0.5f, 0.5f));  // white (set for consistency)
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", glm::vec3(0.3f, 0.2f, 0.9f));
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", commonFocalStrength);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", commonSpecularIntensity);

	// Light 1: blue/white light from raised position
	m_pShaderManager->setVec3Value("lightSources[1].position", glm::vec3(-4.0f, 8.0f, 2.0f));  // Adjusted for sides
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", glm::vec3(0.0f, 0.0f, 0.0f));  // zero to avoid whitewashing everything out
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", glm::vec3(0.2f, 0.2f, 0.8f));
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", glm::vec3(0.8f, 0.7f, 1.0f));
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", commonFocalStrength);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", commonSpecularIntensity);

	// Far away dim light to ensure all sides are lit up
	m_pShaderManager->setVec3Value("lightSources[2].position", glm::vec3(0.0f, -200.0f, 0.0f));  // Very far below
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", glm::vec3(0.0f, 0.0f, 0.0f));
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", glm::vec3(0.0f, 0.0f, 0.0f));
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", glm::vec3(0.0f, 0.0f, 0.0f));
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 1.0f);  // Very weak
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.0f);  // Zero to disable specular

	//  Light 3: Same purpose as Light 2
	m_pShaderManager->setVec3Value("lightSources[3].position", glm::vec3(0.0f, -200.0f, 0.0f)); //very fr below
	m_pShaderManager->setVec3Value("lightSources[3].ambientColor", glm::vec3(0.0f, 0.0f, 0.0f));
	m_pShaderManager->setVec3Value("lightSources[3].diffuseColor", glm::vec3(0.0f, 0.0f, 0.0f));
	m_pShaderManager->setVec3Value("lightSources[3].specularColor", glm::vec3(0.0f, 0.0f, 0.0f));
	m_pShaderManager->setFloatValue("lightSources[3].focalStrength", 1.0f); //very weak
	m_pShaderManager->setFloatValue("lightSources[3].specularIntensity", 0.0f);

}

void SceneManager::LoadSceneTextures()
{
	/*** STUDENTS - add the code BELOW for loading the textures that ***/
	/*** will be used for mapping to objects in the 3D scene. Up to  ***/
	/*** 16 textures can be loaded per scene. Refer to the code in   ***/
	/*** the OpenGL Sample for help.                                 ***/

	// Note: I have copied the "textures" folder from utilities to the solution directory,
	// and I have applied the same textures as in the example picture
	CreateGLTexture("textures/clockface.png", "clockface1"); //a regular clock for the bottom half
	CreateGLTexture("textures/hypno.jpg", "clockface2");	//a hypnotic pattern for the top half
	CreateGLTexture("textures/knobtexture.png", "goldTexture"); //a golden texture for the top bell
	CreateGLTexture("textures/darkgrain.jpg", "handsTexture"); //a dark grained wood for the hands
	CreateGLTexture("textures/rusticwood.jpg", "woodTexture");//added for floor, to match feel of painting
	CreateGLTexture("textures/backdrop.jpg", "backdropTexture"); //Added for backdrop, since painting's sky background too complex
	CreateGLTexture("textures/DisintegrationofPersistence.jpg", "disintegration"); //Added for floor to fit with theme


	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);
		m_pShaderManager->setIntValue(g_UseTwoTexturesName, 0);//to avoid using multiple textures



		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

void SceneManager::SetShaderTwoTextures( //Allows using 2 textures on one shape, based on adding some changes to FragmentShader.glsl
										//based on  https://stackoverflow.com/questions/27345340/how-do-i-render-multiple-textures-in-modern-opengl
	std::string textureTag1,
	std::string textureTag2)
{
	if (m_pShaderManager != NULL)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);
		m_pShaderManager->setIntValue(g_UseTwoTexturesName, 1);

		int textureID1 = FindTextureSlot(textureTag1);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID1);

		int textureID2 = FindTextureSlot(textureTag2);
		m_pShaderManager->setSampler2DValue("objectTexture2", textureID2);
	}

}







/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{

	// load the textures for the 3D scene
	LoadSceneTextures();

	//add textures for lighting usage
	DefineObjectMaterials();
	// add and define the light sources for the scene
	SetupSceneLights();

	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadPlaneMesh();

	// Each type of mesh to be used must be loaded
	// I am adding the following types of mesh in accordance with trying to recreate image 3:
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();

	m_basicMeshes->LoadTorusMesh(torusMinorRadius);


}

/***********************************************************
*  drawClock()
*
*  This method uses combinations of simple 3d shapes to generate a comples shape
* At an arbitrarily chosen point and with an arbitratily chosen rotation and scale
* To understand the groupMatrix method of rotating, note it was based off these stackoverflow posts:
* https://stackoverflow.com/questions/45091505/opengl-transforming-objects-with-multiple-rotations-of-different-axis
* https://stackoverflow.com/questions/33958379/opengl-transform-matrix-order-is-backwards
* https://stackoverflow.com/questions/74110049/precisely-map-world-position-to-uv-texture-coordinates-opengl-compute-shader
* Unfortunately, I could not get the double-texture clockface to work on my own, 
* and could not get some rotations to work, 
* so I had Grok 4 expert code the texture portion and refine the matrix rotation
* I believe the proper way to credit Grok based on
* https://libguides.snhu.edu/c.php?g=92369&p=10173600
* is as follows:
* XAI. (2025). Grok 4 Expert [Large language model]. https://grok.com/
***********************************************************/
void SceneManager::DrawClock(glm::vec3 groupPos, 
						glm::vec3 groupScale, 
						float groupRotX, float groupRotY, float groupRotZ ) {

	groupPos.z = -1 * groupPos.z; //greater Z value should take it back into picture,
								//but default computation takes it more forward
								
	// Compute the group transformation matrix
	glm::mat4 groupMatrix = glm::mat4(1.0f);
	groupMatrix = glm::translate(groupMatrix, groupPos);
	groupMatrix = glm::rotate(groupMatrix, glm::radians(groupRotX), glm::vec3(1.0f, 0.0f, 0.0f));
	groupMatrix = glm::rotate(groupMatrix, glm::radians(groupRotY), glm::vec3(0.0f, 1.0f, 0.0f));
	groupMatrix = glm::rotate(groupMatrix, glm::radians(groupRotZ), glm::vec3(0.0f, 0.0f, 1.0f));
	groupMatrix = glm::scale(groupMatrix, groupScale);// Local variables for the clock (relative to group origin)

	float PAINT_MAX = 255.0f; // To allow getting colors from Microsoft Paint's 0-255 RGB scale
	// Achieve gold coloring
	float rimR = 249.0f / PAINT_MAX;
	float rimG = 176.0f / PAINT_MAX;
	float rimB = 26.0f / PAINT_MAX;

	// Clock rim parameters
	float clockCenterX = 0.0f; // Relative to groupPos (shifted from original -1 to center)
	float clockCenterY = 0.0f; // Relative to groupPos (shifted from original 2 to center)
	float clockRimRadius = 1.2f;
	float torusMinorRadius = 0.1f; // adjusted to match DrawTorusMesh() minor radius 
	float squished = 0.1f; // Low depth value to give squished appearance

	// Draw clock rim
	glm::vec3 scaleXYZ = glm::vec3(clockRimRadius, clockRimRadius, squished);
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(clockCenterX, clockCenterY, 0.0f);

	// Compute local model matrix
	glm::mat4 localModel = glm::translate(glm::mat4(1.0f), positionXYZ);
	localModel = glm::rotate(localModel, glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	localModel = glm::rotate(localModel, glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	localModel = glm::rotate(localModel, glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	localModel = glm::scale(localModel, scaleXYZ);

	// Apply group matrix and set to shader
	glm::mat4 fullModel = groupMatrix * localModel;
	m_pShaderManager->setMat4Value(g_ModelName, fullModel);

	SetShaderColor(rimR, rimG, rimB, 1.0f);
	SetShaderTexture("goldTexture");
	m_basicMeshes->DrawTorusMesh();

	// Clock face - Adjusted radius to better fill the rim (subtract minor radius for inner fit)
	float clockFaceRadius = clockRimRadius - torusMinorRadius;
	scaleXYZ = glm::vec3(clockFaceRadius, clockFaceRadius, squished);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(clockCenterX, clockCenterY, 0.0f);

	// Local model
	localModel = glm::translate(glm::mat4(1.0f), positionXYZ);
	localModel = glm::rotate(localModel, glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	localModel = glm::rotate(localModel, glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	localModel = glm::rotate(localModel, glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	localModel = glm::scale(localModel, scaleXYZ);

	fullModel = groupMatrix * localModel;
	m_pShaderManager->setMat4Value(g_ModelName, fullModel);

	// Set objectPosition and objectScale for UV calculation in shader (critical for two-texture split)
	// objectPosition: full world position of the clock face center
	glm::vec3 fullPosition = groupPos + positionXYZ;
	m_pShaderManager->setVec3Value("objectPosition", fullPosition);

	// objectScale: full effective scale (group * local; use xy for 2D UV, z not needed)
	glm::vec3 fullScale = groupScale * glm::vec3(clockFaceRadius, clockFaceRadius, squished);
	m_pShaderManager->setVec3Value("objectScale", fullScale);

	// Set UVscale to 1.0 to avoid tiling (stretch to fit)
	m_pShaderManager->setVec2Value("UVscale", glm::vec2(1.0f, 1.0f));  // Explicitly set to prevent tiling

	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	SetShaderTwoTextures("clockface2", "clockface1");  // clockface2 for top, clockface1 for bottom
	m_basicMeshes->DrawSphereMesh();

	// Clock hands
	float clockHandLength = clockRimRadius; // Long hands going to the edge of the clock
	float clockHandY = clockCenterY + 0.0f * clockHandLength; // Start bottom of hand at center of clockface
	// (0.0 for cone; use 0.5 for box if switching)

// First hand (minute)
	scaleXYZ = glm::vec3(squished, clockHandLength, squished);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = -330.0f; // The 55 minutes position
	positionXYZ = glm::vec3(clockCenterX, clockHandY, squished); // Slightly positive Z so hand is in front

	localModel = glm::translate(glm::mat4(1.0f), positionXYZ);
	localModel = glm::rotate(localModel, glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	localModel = glm::rotate(localModel, glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	localModel = glm::rotate(localModel, glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	localModel = glm::scale(localModel, scaleXYZ);

	fullModel = groupMatrix * localModel;
	m_pShaderManager->setMat4Value(g_ModelName, fullModel);

	SetShaderColor(0.0f, 0.0f, 0.0f, 1.0f);
	SetShaderTexture("handsTexture");
	m_basicMeshes->DrawConeMesh();

	// Second clock hand (shorter, hour hand)
	scaleXYZ = glm::vec3(squished, 0.75f * clockHandLength, squished);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = -210.0f; // The 7 o'clock position
	positionXYZ = glm::vec3(clockCenterX, clockHandY, squished);

	localModel = glm::translate(glm::mat4(1.0f), positionXYZ);
	localModel = glm::rotate(localModel, glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	localModel = glm::rotate(localModel, glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	localModel = glm::rotate(localModel, glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	localModel = glm::scale(localModel, scaleXYZ);

	fullModel = groupMatrix * localModel;
	m_pShaderManager->setMat4Value(g_ModelName, fullModel);

	SetShaderColor(0.0f, 0.0f, 0.0f, 1.0f);
	SetShaderTexture("handsTexture");
	m_basicMeshes->DrawConeMesh();

	// Bell at top
	float bellHeight = 0.3f;
	float bellWidth = 0.5f;
	float bellDepth = 0.25f; // The bell doesn't look as squished in the painting
	float clockAndRimHeight = clockCenterY + clockRimRadius;
	scaleXYZ = glm::vec3(bellWidth, bellHeight, bellDepth);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	float bellPositionY = clockAndRimHeight + bellHeight / 2.0f;  // Adjusted to center the bell on top
	positionXYZ = glm::vec3(clockCenterX, bellPositionY, 0.0f);

	localModel = glm::translate(glm::mat4(1.0f), positionXYZ);
	localModel = glm::rotate(localModel, glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	localModel = glm::rotate(localModel, glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	localModel = glm::rotate(localModel, glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	localModel = glm::scale(localModel, scaleXYZ);

	fullModel = groupMatrix * localModel;
	m_pShaderManager->setMat4Value(g_ModelName, fullModel);

	SetShaderColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow. While it's the same shade in the painting, this makes them easier to tell apart
	SetShaderTexture("goldTexture");
	m_basicMeshes->DrawSphereMesh();




}





/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh (the floor)
	//positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);


	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	// set the color values into the shader
	//Set color as fraction of 255 to allow compatability with Paint color dropper tool
	//SetShaderColor(95. / 255., 124. / 255., 200. / 255., 1);; //Note to self: this is the floor color
	SetShaderTexture("backdropTexture"); //Add interesting background on floor according to theme
	SetShaderMaterial("glass"); //Make floor unusually shiny, like glass, for artstic effect

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	//scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
	scaleXYZ = glm::vec3(20.0f, 8.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 7.0f, -10.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color values into the shader
	//SetShaderColor(0, 0, 1, 1); //Note to self: this is the back wall color
	SetShaderTexture("disintegration"); //add artistic background instead of sky
	SetShaderMaterial("glass"); //create midnight blue color to reflect on clock


	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/
	// ADDITION OF NEW SHAPES BEGINS HERE
	/****************************************************************/



	glm::vec3 mainClockPosition = glm::vec3(-1, 2, 0);

	glm::vec3 smallClockScale = glm::vec3(.2, .2, .1);
	glm::vec3 smallClockPosition = glm::vec3(-1, 4, 0);


	glm::vec3 largelockPosition = glm::vec3(1, 3.5, 2);
	glm::vec3 largeClockScale = glm::vec3(4, 2, 1); //oblong shape to match painting

	glm::vec3 distortedClockPosition = glm::vec3(-4, 2, -2);
	glm::vec3 distortedClockScale = glm::vec3(1, 1, 2);
	glm::vec3 distortedClockRotationDeg = glm::vec3(-50, 0, 90);

	DrawClock(mainClockPosition, glm::vec3(1, 1, 1), 0, 0, 0);

	DrawClock(largelockPosition, largeClockScale, 0, -30, 0); //rotate -30 degrees to point slightly to main clock

	DrawClock(distortedClockPosition, distortedClockScale,
		distortedClockRotationDeg.x, distortedClockRotationDeg.y, distortedClockRotationDeg.z);
	DrawClock(smallClockPosition, smallClockScale, 0, 0, 0);





}