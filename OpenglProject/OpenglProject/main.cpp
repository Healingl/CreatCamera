#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "RenderObject.hpp"
#include "shader.h"
#include "RayOBB.h"

GLFWwindow* window;
GLuint const screenWidth = 640, screenHeight = 480;

GLuint program;
GLuint pickProgram;
GLuint mouseX, mouseY;
GLuint lastMouseX, lastMouseY;

RenderObject cube1, cube2;
glm::vec3 cameraPosition(0.7f, 1.0f, 4.0f);
glm::vec3 cameraPositionSaved;
glm::vec3 cameraFront = glm::vec3(-0.3f, -0.3f, -1.0f);
glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
glm::mat4 view;
glm::mat4 projection;
// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame
bool keys[1024];


bool rotateView = false;
bool drawLines = false;

void do_movement()
{
	// Camera controls
	GLfloat cameraSpeed = 5.0f * deltaTime;
	if (keys[GLFW_KEY_W])
		cameraPosition += cameraSpeed * cameraFront;
	if (keys[GLFW_KEY_S])
		cameraPosition -= cameraSpeed * cameraFront;
	if (keys[GLFW_KEY_A])
		cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (keys[GLFW_KEY_D])
		cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

}

int init_resources()
{
	// Depth Testing 
	glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_ALWAYS);		// ignore depth testing


	// Blending 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// Face Culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// Load Shaders
	program = LoadShader("vertex.vert", "fragment.frag");
	pickProgram = LoadShader("pickVertex.vert", "pickFragment.frag");



	GLfloat vertices[] = {
		// vertex postion		    // vertex color
		// x        y       z       // r      g       b       a
		-0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f,		//0            
		0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f,		//1            
		0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f,		//2            7-------6 
		-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,	//3            |\      |\	
		-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f, 1.0f,		//4            | 4-----|-5
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f,		//5            3-|-----2 |
		0.5f, 0.5f, -0.5f, 0.4f, 0.2f, 0.8f, 1.0f,		//6             \|      \|
		-0.5f, 0.5f, -0.5f, 0.8f, 0.4f, 0.0f, 1.0f		//7              0-------1
	};

	GLuint indices[] = {
		0, 1, 4, 1, 5, 4,
		1, 2, 5, 2, 6, 5,
		2, 3, 6, 3, 7, 6,
		3, 0, 7, 0, 4, 7,
		1, 0, 2, 3, 2, 0,
		4, 5, 7, 5, 6, 7,
	};


	// Create our cubes
	cube1 = RenderObject(1);
	cube1.BindMesh_p3_c4(vertices, sizeof(vertices), indices, sizeof(indices));
	cube2 = RenderObject(2);
	cube2.BindMesh_p3_c4(vertices, sizeof(vertices), indices, sizeof(indices));


	// Setup Projection Matrix
	projection = glm::perspective(45.0f, (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);


	return 1;
}

void onDisplay()
{
	// Setup View Matrix
	view = glm::lookAt(cameraPosition, cameraPosition+cameraFront, cameraUp);

	// Setup each Model Matrix
	cube1.SetModel(glm::translate(glm::mat4(), glm::vec3(0.3f, 0.0f, -2.0f)));
	cube2.SetModel(glm::translate(glm::mat4(), glm::vec3(-0.3f, 0.0f, 1.0f)));


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// draw the behind cube 
	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(cube1.model));
	if (drawLines)
		cube1.Draw_i(GL_LINE_LOOP);
	else
		cube1.Draw_i();


	// draw the front cube 
	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(cube2.model));
	if (drawLines)
		cube2.Draw_i(GL_LINE_LOOP);
	else
		cube2.Draw_i();

	if (drawLines)
		DrawLine(rayBegin, rayEnd, program);
}


void free_resources()
{
	glDeleteProgram(program);
}

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

// Callback Function: get the mouse position, called when mouse moved
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	mouseX = (int)xpos;
	mouseY = screenHeight - (int)ypos;

}

// Callback Function: called when mouse clicked
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		drawLines = !drawLines;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		DoPick(mouseX,mouseY,screenWidth,screenHeight,view,projection,cube1);
		DoPick(mouseX, mouseY, screenWidth, screenHeight, view, projection, cube2);
	}
}

int main(void)
{
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(screenWidth, screenHeight, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK)
	{
		//      throw std::runtime_error("glew fails to start.");
		fprintf(stderr, "glew error\n");
	}

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	GLint maxV;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxV);
	fprintf(stderr, "maxv: %d\n", maxV);

	init_resources();

	while (!glfwWindowShouldClose(window))
	{
		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		do_movement();
		onDisplay();
		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	free_resources();

	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}
