#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  

int main(){
	GLFWwindow* window = NULL;
	if(!glfwInit()){
		return -1;
	}
	window = glfwCreateWindow(800, 600, "Renderer", NULL, NULL);
	if(!window){
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glViewport(0, 0, 800, 600);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glewInit();

	unsigned int buffer;
	glGenBuffers(1, &buffer);
	while(!glfwWindowShouldClose(window)){
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}
