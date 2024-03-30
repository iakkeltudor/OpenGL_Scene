//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>

int glWindowWidth = 1920;
int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 16384;
const unsigned int SHADOW_HEIGHT = 16384;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;

gps::Camera myCamera(
				glm::vec3(0.0f, 2.0f, 5.5f), 
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.1f;

bool pressedKeys[1024];
GLfloat lightAngle;

gps::Model3D tank;
gps::Model3D tankHull;
gps::Model3D tankTurret;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D bush;
gps::Model3D tree;
gps::Model3D tree2;
gps::Model3D tree3;
gps::Model3D treeTrunk;
gps::Model3D lamp;
gps::Model3D house;
gps::Model3D crashedTank1;
gps::Model3D crashedTank2;
gps::Model3D crashedTank3;
gps::Model3D crashedTank4;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader skyboxShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

gps::SkyBox mySkyBox;

bool showDepthMap;

int fogOrNot = 0;
float fogDensity = 0.05f;
GLint fogOrNotLoc;

float tankMove1;
float tankMove2;

int spotLighting = 1;
float spotlight;
float spotlight2;
glm::vec3 spotLightDirection;
glm::vec3 spotLightPosition;

float deltaTime = 0.0f;
float intAngle;

int wireFrame = 0;

GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);

	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	//if (key == GLFW_KEY_M && action == GLFW_PRESS)
	//	showDepthMap = !showDepthMap;

	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		if (fogOrNot == 0) {
			fogOrNot = 1;
		}
		else {
			fogOrNot = 0;
		}
		myCustomShader.useShaderProgram();
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);
		fogOrNotLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogOrNot");
		glUniform1i(fogOrNotLoc, fogOrNot);
	}

	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		if (wireFrame == 0) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			wireFrame = 1;
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			wireFrame = 0;
		}
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		fogDensity += 0.01f;
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);
	}

	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		fogDensity -= 0.01f;
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

float lastX;
float lastY;
bool mouse = true;
float yaw = -90.0f, pitch;

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (mouse)
	{
		lastX = xpos;
		lastY = ypos;
		mouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.2f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
}

GLfloat angle;

void processMovement() {

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	//tank moves front
	if (pressedKeys[GLFW_KEY_UP]) {
		if (tankMove1 < 100)
			tankMove1 += 0.1;
	}

	//tank moves back
	if (pressedKeys[GLFW_KEY_DOWN]) {
		if (tankMove1 < 100)
			tankMove1 -= 0.1;
	}

	//tank moves left
	if (pressedKeys[GLFW_KEY_RIGHT]) {
		if (tankMove2 < 100)
			tankMove2 -= 0.1;
	}

	//tank moves right
	if (pressedKeys[GLFW_KEY_LEFT]) {
		if (tankMove2 < 100)
			tankMove2 += 0.1;
	}

}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    
    //window scaling for HiDPI displays
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    //for sRBG framebuffer
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    //for antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);
	//initMouseCallback();

	glfwSwapInterval(1);

#if not defined (__APPLE__)
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	tank.LoadModel("objects/tankyTank/finalTank.obj");
	tankHull.LoadModel("objects/tankyTank/finalTankHull.obj");
	tankTurret.LoadModel("objects/tankyTank/finalTankTurret.obj");
	ground.LoadModel("objects/ground/ground.obj");
	lightCube.LoadModel("objects/cube/cube.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
	bush.LoadModel("objects/bush/bush.obj");
	tree.LoadModel("objects/tree/tree.obj");
	tree2.LoadModel("objects/tree/tree2.obj");
	tree3.LoadModel("objects/tree/tree3.obj");
	treeTrunk.LoadModel("objects/tree/treeTrunk.obj");
	lamp.LoadModel("objects/lamppost/Street_Lamp_1.obj");
	house.LoadModel("objects/house/cottage_obj.obj");
	crashedTank1.LoadModel("objects/crashedTanks/IS-3/IS-3_CRASHED.obj");
	crashedTank2.LoadModel("objects/crashedTanks/Tiger-II/Tiger-2_CRASHED.obj");
	crashedTank3.LoadModel("objects/crashedTanks/ST-II/ST-II_CRASHED.obj");
	crashedTank4.LoadModel("objects/crashedTanks/Grille-15/Grille-15_CRASHED.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/shadow.vert", "shaders/shadow.frag");
	depthMapShader.useShaderProgram();

	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 30.0f, 8.0f);
	//lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");	
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	//spot light
	spotlight = glm::cos(glm::radians(0.0f));

	spotLightDirection = glm::vec3(0, -1, 0);
	spotLightPosition = glm::vec3(2.0f, 3.05f, 1.0f);

	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "spotlight"), spotlight);
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLightDirection"), 1, glm::value_ptr(spotLightDirection));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLightPosition"), 1, glm::value_ptr(spotLightPosition));

	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "spotLighting"), spotLighting);
	
	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initFBO() {
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix

	//lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	//glm::mat4 lightView = glm::lookAt(glm::vec3(lightRotation * glm::vec4(lightDir, 1.0f)), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 20.0f, far_plane = 150.0f;
	glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}

void initSkybox() {
	std::vector<const GLchar*> faces;
	faces.push_back("clouds1/clouds1_east.bmp");
	faces.push_back("clouds1/clouds1_west.bmp");
	faces.push_back("clouds1/clouds1_up.bmp");
	faces.push_back("clouds1/clouds1_down.bmp");
	faces.push_back("clouds1/clouds1_north.bmp");
	faces.push_back("clouds1/clouds1_south.bmp");

	//faces.push_back("skybox/right.tga");
	//faces.push_back("skybox/left.tga");
	//faces.push_back("skybox/top.tga");
	//faces.push_back("skybox/bottom.tga");
	//faces.push_back("skybox/back.tga");
	//faces.push_back("skybox/front.tga");
	mySkyBox.Load(faces);
}

void drawObjects(gps::Shader shader, bool depthPass) {
		
	shader.useShaderProgram();
	
	model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(0.0f + tankMove2, 0.0f, 0.0f + tankMove1));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	//tank.Draw(shader);
	//shader.useShaderProgram();
	//glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tank.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	ground.Draw(shader);	


	//model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, -0.4f, 0.0f));
	//model = glm::scale(model, glm::vec3(0.5f));
	//glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	//pathway.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, -0.5f, -6.0f));
	model = glm::scale(model, glm::vec3(0.25f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	bush.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(2.5f, -0.5f, -5.0f));
	model = glm::scale(model, glm::vec3(0.25f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	bush.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, -0.5f, -1.0f));
	model = glm::scale(model, glm::vec3(0.25f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	bush.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, -0.5f, -2.0f));
	model = glm::scale(model, glm::vec3(0.25f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	bush.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, -0.5f, 14.0f));
	model = glm::scale(model, glm::vec3(0.25f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	bush.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-15.0f, -0.5f, 3.0f));
	model = glm::scale(model, glm::vec3(0.25f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	bush.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, -0.5f, 4.0f));
	model = glm::scale(model, glm::vec3(0.25f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	bush.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, -0.5f, 3.0f));
	model = glm::scale(model, glm::vec3(0.25f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	bush.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.5f, -0.5f, -1.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-5.5f, -0.5f, 7.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(4.5f, -0.5f, 2.5f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(8.5f, -0.5f, -2.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(12.0f, -0.5f, -11.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.5f, -0.5f, -3.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree2.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-6.5f, -0.5f, -8.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree2.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.5f, -0.5f, 12.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree2.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(6.5f, -0.5f, 12.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree2.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(12.5f, -0.5f, 12.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree2.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-12.5f, -0.5f, -2.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree2.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-15.5f, -0.5f, 4.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree3.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(4.5f, -0.5f, 17.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree3.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(9.5f, -0.5f, 13.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree3.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-20.5f, -0.5f, -7.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree3.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-11.5f, -0.5f, -1.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree3.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(8.5f, -0.5f, 0.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree3.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-5.5f, -0.5f, 9.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tree3.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-7.5f, -0.5f, 0.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	treeTrunk.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, -0.5f, 14.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	treeTrunk.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(13.5f, -0.5f, 4.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	treeTrunk.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(7.5f, -0.5f, 0.0f));
	model = glm::scale(model, glm::vec3(0.4f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	treeTrunk.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -0.5f, 1.0f));
	model = glm::scale(model, glm::vec3(1.25f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	lamp.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-13.0f, -0.5f, 12.0f));
	model = glm::scale(model, glm::vec3(0.25f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	house.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(15.0f, -0.5f, -18.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.25f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	house.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(18.0f, -0.5f, 21.0f));
	model = glm::scale(model, glm::vec3(0.25f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	house.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-9.0f, 0.0f, 3.0f));
	model = glm::scale(model, glm::vec3(0.8f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	crashedTank1.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(8.0f, 0.0f, 14.0f));
	model = glm::rotate(model, glm::radians(100.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.8f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	crashedTank1.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-7.0f, 0.0f, 17.0f));
	model = glm::rotate(model, glm::radians(25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.1f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	crashedTank2.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(7.0f, 0.0f, -18.0f));
	model = glm::rotate(model, glm::radians(200.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.1f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	crashedTank2.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(7.0f, 0.0f, 5.0f));
	model = glm::rotate(model, glm::radians(65.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.8f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	crashedTank3.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-18.0f, 0.0f, -2.0f));
	model = glm::rotate(model, glm::radians(130.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.8f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	crashedTank3.Draw(shader);


	model = glm::translate(glm::mat4(1.0f), glm::vec3(-6.0f, 0.0f, 13.0f));
	model = glm::rotate(model, glm::radians(130.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.6f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	crashedTank4.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-13.0f, 0.0f, -12.0f));
	model = glm::rotate(model, glm::radians(175.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.6f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	crashedTank4.Draw(shader);

	//"animation"
	float rotationSpeed = 25.0f;
	deltaTime = crashedTank4.update();
	intAngle += rotationSpeed * deltaTime;

	if (intAngle >= 360.0f) {
		intAngle -= 360.0f;
	}

	model = glm::translate(glm::mat4(1.0f), glm::vec3(25.0f, 0.0f, -25.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tankHull.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(25.0f, 0.035f, -25.1f));
	model = glm::rotate(model, glm::radians(intAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	tankTurret.Draw(shader);

	if (!depthPass) {
		mySkyBox.Draw(skyboxShader, view, projection);
	}
}

void renderScene() {

	processMovement();

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		//draw a white cube around the light

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = lightRotation;
		model = glm::translate(model, 1.0f * lightDir);
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		lightCube.Draw(lightShader);
	}
}

void cleanup() {
	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initSkybox();
	initUniforms();
	initFBO();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
