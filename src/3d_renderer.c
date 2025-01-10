#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cglm/cglm.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  

static void processInput(GLFWwindow *window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
	}
}

const char* vertexShaderPath = "src/shaders/vertexShader.glsl";
const char* fragmentShaderPath = "src/shaders/fragmentShader.glsl";

int logShaderCompileErrors(GLuint shader){
	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success){
		char infoLog[1024];
		glGetShaderInfoLog(shader, 1024, NULL, infoLog);
		printf("Shader Error: %s\n", infoLog);
	}
	return success;
}

typedef struct{
	char* data;
	int width;
	int height;
}BMPImage;

int loadBMPImage(const char* path, BMPImage* image){ // loads texture data into currently bound texture
	// main texture related stuff
	int fd = open(path, O_RDONLY, S_IRUSR);
	struct stat sb;
	if(fstat(fd, &sb) == -1){
		printf("Error loading file\n");
		// do error stuff
		// close file descripter
	}
	char* data = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	unsigned int width, height, offset, imageSize;
	if(data[0] != 'B' || data[1] != 'M'){
		// not correct file type
		printf("Invalid image type\n");
	}
	// cause we are reading this bytewise we need to cast the data
	width = *(int*)&data[0x12]; // is two bytes but should be ok like this?
	height = *(int*)&data[0x16];
	offset = *(int*)&data[0x0A];
	imageSize = *(int*)&data[0x22];
	// set default offset if needed
	if(offset == 0) offset = 54;
	if(imageSize == 0) imageSize = width * height * 3;
	// copy data to new array
	char* colorData = malloc(imageSize);
	memcpy(colorData, data+offset, imageSize);
	image->data = colorData;
	image->width = width;
	image->height = height;
	munmap(data, sb.st_size);
	close(fd);

	return 0;
};

float vertices[] = {
    // positions          // colors           // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
};

unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};

#pragma pack(push, 1)
typedef struct {
	float x;
	float y;
	float z;
}Vertex;

typedef struct {
	float u;
	float v;
}Normal;

typedef struct { // vertex indices
	unsigned int v1;
	unsigned int v2;
	unsigned int v3;
}Face;
#pragma pack(pop)

/*
typedef struct {
	Vertex* vertices;
	unsigned int verticesLen;
	Normal* normals;
	unsigned int normalsLen;
	Face* faces;
	unsigned int facesLen;
	//texture coordinates[]
	unsigned int materialIndex;
}Mesh;

typedef struct node {
	struct node* children;
	unsigned int childrenLen;
	Mesh* meshes;
	unsigned int meshesLen;
}Node;

typedef struct { //floats or ints?
	float r;
	float g;
	float b;
}Color;

typedef struct { // TODO add texture maps for properties in addition to defaults
	Color diffuseColor;
	Color specularColor;
	Color ambientColor;
	Color emissiveColor;
	Color transparentColor;
	Color reflectiveColor;
	// TODO add normal maps eventually
	float reflectivity;
	float opacity;
	float shininess;
	float shininessStrength;
}Material;

struct {
	Node* rootNode;
	Mesh* meshes;
	unsigned int meshesLen;
	Material* materials;
	unsigned int materialsLen;
}Scene;
*/

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

	/*

    GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
    GL_STATIC_DRAW: the data is set only once and used many times.
    GL_DYNAMIC_DRAW: the data is changed a lot and used many times.
	*/

	struct stat sb;
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	int fd = open(vertexShaderPath, O_RDONLY, S_IRUSR);
	if(fstat(fd, &sb)==-1){
		printf("failed to load vertex shader source\n");
	}
	char* shaderSource = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	glShaderSource(vertexShader, 1, (const char**)&shaderSource, NULL);
	munmap(shaderSource, sb.st_size);
	close(fd);
	glCompileShader(vertexShader);

	logShaderCompileErrors(vertexShader);

	//TODO add logging

	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	fd = open(fragmentShaderPath, O_RDONLY, S_IRUSR);
	if(fstat(fd, &sb)==-1){
		printf("failed to load fragment shader source\n");
	}
	shaderSource = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	glShaderSource(fragmentShader, 1, (const char**)&shaderSource, NULL);
	munmap(shaderSource, sb.st_size);
	close(fd);
	glCompileShader(fragmentShader);

	logShaderCompileErrors(fragmentShader);

	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);
	
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
	// color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coordinate
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);


	// Creating a texture with stb image
	int width, height, nrChannels;
	BMPImage imageData;
	loadBMPImage("src/textures/fox.bmp", &imageData); // loads texture data into currently bound texture
	//unsigned char *imageData = stbi_load("src/fox.jpg", &width, &height, &nrChannels, 0); 
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, imageData);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageData.width, imageData.height, 0, GL_BGR, GL_UNSIGNED_BYTE, imageData.data);
	glGenerateMipmap(GL_TEXTURE_2D);
	free(imageData.data);

	while(!glfwWindowShouldClose(window)){
		processInput(window);
		// rendering commands here
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		// bind texture
		glBindTexture(GL_TEXTURE_2D, texture);
		// render container
		glUseProgram(shaderProgram);
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//		glBindVertexArray(0);
		// draw buffer swap
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}
