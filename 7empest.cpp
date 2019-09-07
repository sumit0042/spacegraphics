#include<GL/glew.h>     // use this before GLFW
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "Camera.h"
#include "Shader.h"
#include "Cube.h"

GLuint loadBMP_custom(const char* imagepath);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}


int main()
{
	GLFWwindow* window;

	// Initialize the library 
	if (!glfwInit())
		return -1;

	// Create a windowed mode window and its OpenGL context 
	window = glfwCreateWindow(1280, 720, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, 800, 600);
	// Make the window's context current 
	glfwMakeContextCurrent(window);

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//create_faces_data();

	glewInit();

	Cube bigBox(0.2f, 0.2f, 0.2f, 0);
	std::vector<float> objCol;
	objCol = { 0.0f, 0.0f, 0.9f };
	bigBox.bind_obj_color(objCol);
	/*Cube smlBox(0.09f, 0.09f, 0.09f, 0);
	smlBox.bind_obj_color({0.0f, 0.9f, 0.0f});
	smlBox.translate(0.4f, 0.4f, -0.9f);*/

	glActiveTexture(0);

	float ang = 0.0f;
	float r = 0.9f;

	// Loop until the user closes the window 
	while (!glfwWindowShouldClose(window))
	{
			ang += 0.01f;
		
		camera.Front = { -r * cos(ang), 0.0f, -r * sin(ang) };
		camera.Position = {r*cos(ang), 0.0f, r * sin(ang) };
		bigBox.m_lightSrc = { r * cos(ang), 0.0f, r * sin(ang) };
		glEnable(GL_DEPTH_TEST);
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			r -= 0.01;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			r += 0.01;
		if (r < 0.1)
			r = 0.1f;
		//processInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		bigBox.draw(camera);
		/*smlBox.draw(camera);*/

		glUseProgram(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glUseProgram(0);
	glfwTerminate();
	return 0;
}


GLuint loadBMP_custom(const char* imagepath)
{
	unsigned char header[54]; // Each BMP file begins by a 54-bytes header
	unsigned int dataPos;     // Position in the file where the actual data begins
	unsigned int width, height;
	unsigned int imageSize;   // = width*height*3
	// Actual RGB data
	unsigned char* data;

	FILE* file = fopen(imagepath, "rb");
	if (!file)
	{
		printf("Image could not be opened\n"); return 0;
	}
	if (fread(header, 1, 54, file) != 54) { // If not 54 bytes read : problem
		printf("Not a correct BMP file\n");
		return false;
	}

	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;
	}

	// Read ints from the byte array
	dataPos = *(int*) & (header[0x0A]);
	imageSize = *(int*) & (header[0x22]);
	width = *(int*) & (header[0x12]);
	height = *(int*) & (header[0x16]);
	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = width * height * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way
	// Create a buffer
	data = new unsigned char[imageSize];
	// Read the actual data from the file into the buffer
	fread(data, 1, imageSize, file);
	//Everything is in memory now, the file can be closed
	fclose(file);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	delete[] data;
	return textureID;
}