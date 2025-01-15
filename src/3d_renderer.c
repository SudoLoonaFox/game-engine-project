#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cglm/cglm.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Specified in models.csv id,name,meshpath,texturepath
const char* MODEL_IN_FORMAT = "%d,%[^,],%[^,],%[^,]";

// will have multiple ones for different rendering types
const char* vertexShaderPath = "src/shaders/vertexShader.glsl";
const char* fragmentShaderPath = "src/shaders/fragmentShader.glsl";



typedef struct{
	unsigned int id;
	char name[30];
	char meshPath[30];
	char texturePath[30];
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	unsigned int indicesNo;
	unsigned int texture;
}Model;

typedef struct{
	char* data;
	int width;
	int height;
}BMPImage;

// TODO make camera struct
typedef struct{
	char name[20];
}Camera;

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
	float r;
	float g;
	float b;
	float u;
	float v;
}Vertex;

typedef struct { // vertex indices
	unsigned int v1;
	unsigned int v2;
	unsigned int v3;
}Face;
#pragma pack(pop)
//TODO finish this
/*
void lookAt(GLFWwindow* window){
	int mouseX, mouseY;
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glfwGetCursorPos(&mouseX, &mouseY);
	glfwSetCursorPos(width/2, height/2);
	printf("%d %d\n", mouseX, mouseY);
}
*/

// Needs to take in the data and lengths of data and turn it into the buffers and store the buffers in the model
// return new or modify?

//TODO add texture here? or elsewhere
Model* dataToBuffers(Model* model, Vertex* vertices, unsigned int verticesLen, unsigned int* indices, unsigned int indicesLen){
	model->indicesNo = indicesLen;
    glGenVertexArrays(1, &model->VAO);
    glGenBuffers(1, &model->VBO);
    glGenBuffers(1, &model->EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(model->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, model->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*verticesLen, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indicesLen, indices, GL_STATIC_DRAW);

	// position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (void*)0);
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
	return model;
}
	

//TODO change this to load models based on scene csv
Model* loadModelsIndex(int* length){
	char* path = "src/models.csv";
	FILE* file = fopen(path, "r");
	if(file == NULL){
		return NULL;
	}
	//TODO add reallocation
	Model* models = malloc(sizeof(Model)*10);
	char line[128];
	int modelIndex = 0;
	while(1){
		if(fgets(line, 128, file)==NULL){
			break;
		}
		//printf("%s\n", line);
		if(0==sscanf(line, "%d,%[^,],%[^,],%[^,]", &models[modelIndex].id, models[modelIndex].name, models[modelIndex].meshPath, models[modelIndex].texturePath)){
			continue;
		}
		modelIndex++;
	}
	fclose(file);
	*length = modelIndex;
	for(int i = 0; i<modelIndex; i++){
		printf("id: %d, name: %s, path: %s\n", models[i].id, models[i].name, models[i].meshPath);
	}
	return models;
}
void getModelFromIndex(int id, Model* model);

void loadModelObj(Model* model){
	const int DEFAULT_SIZE = 500000;
	// check if meshpath is set
	/*
	if(model->meshPath==NULL){
		return NULL;
	}
	*/
	FILE* file = fopen(model->meshPath, "r");
	if(file == NULL){
		return;
	}
	//TODO Add reallocation
	unsigned int verticesLen = 0;
	unsigned int textureCoordinatesLen = 0;
	unsigned int vertexNormalsLen = 0;
	unsigned int faceIndex = 0;
	Vertex* vertices = malloc(sizeof(Vertex)*DEFAULT_SIZE);
	float* textureCoordinates = malloc(sizeof(float)*2*DEFAULT_SIZE);
	//Normal* vertexNormals = malloc(sizeof(VertexNormal)*DEFAULT_SIZE);
	//Face* faces = malloc(sizeof(Face)*DEFAULT_SIZE);
	unsigned int* indices = malloc(sizeof(unsigned int)*DEFAULT_SIZE*3);
	// TODO Remake to work line by line and can scan multiple ways
	// IMPORTANT: obj indexing starts at 1
	//char startSymbol[20];
	char line[128];
	while(fgets(line, 128, file)){
		printf("Line: %s\n", line);

		if(line[0] == 'v' && line[1] == ' '){
			sscanf(line, "v %f %f %f\n", &vertices[verticesLen].x, &vertices[verticesLen].y, &vertices[verticesLen].z);
			verticesLen++;
			printf("Vertex Added NO %i\n", verticesLen);
		}
		else if(line[0] == 'v' && line[1] == 't'){
			sscanf(line, "vt %f %f\n", &textureCoordinates[2*textureCoordinatesLen], &textureCoordinates[2*textureCoordinatesLen+1]);
			printf("Texture\n");
			textureCoordinatesLen++;
		}
		/*
		else if(strcmp(startSymbol, "vn") == 0){
			sscanf(line, "vn %f %f %f\n", vertexNormal[vertexNormalLen].x, vertexNormal[vertexNormalLen].y, vertexNormal[vertexNormalLen].z);
			vertexNormalLen++;
		}
		*/
		else if(line[0] == 'f' && line[1] == ' '){
			// possible formats are:
			// f v v v
			// f v/vt v/vt v/vt
			// f v/vt/vn v/vt/vn v/vt/vn
			// f v//vn v//vn v//vn
			if(3 == sscanf(line, "f %d %d %d", &indices[faceIndex*3], &indices[faceIndex*3+1], &indices[faceIndex*3+2])){
				// obj starts at 1 not 0 index
				// can I do &indices[faceIndex*3]-- ? or something
				indices[faceIndex*3]--;
				indices[faceIndex*3+1]--;
				indices[faceIndex*3+2]--;
				faceIndex++;
				printf("f d d d");
				continue;
			}
			int vt[3];
			int vn[3];
			if(9 == sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
						&indices[faceIndex*3], &vt[0], &vn[0],
						&indices[faceIndex*3+1], &vt[1], &vn[1],
						&indices[faceIndex*3+2], &vt[2], &vn[2])){
				// obj starts at 1 not 0 index
				// can I do &indices[faceIndex*3]-- ? or something
				indices[faceIndex*3]--;
				indices[faceIndex*3+1]--;
				indices[faceIndex*3+2]--;
				//TODO add vertex normal stuff

				// set the texture coordinate stored in vertex
				vertices[indices[faceIndex*3]].u = textureCoordinates[2*vt[0]];
				vertices[indices[faceIndex*3]].v = textureCoordinates[2*vt[0]+1];

				vertices[indices[faceIndex*3+1]].u = textureCoordinates[2*vt[1]];
				vertices[indices[faceIndex*3+1]].v = textureCoordinates[2*vt[1]+1];

				vertices[indices[faceIndex*3+2]].u = textureCoordinates[2*vt[2]];
				vertices[indices[faceIndex*3+2]].v = textureCoordinates[2*vt[2]+1];

				faceIndex++;
				continue;
			}
				//printf("f d/d/d d/d/d d/d/d");
		}
	}
	int indiceLen = faceIndex*3;
	dataToBuffers(model, vertices, verticesLen, indices, indiceLen);
	// TODO all that freeing stuff
	fclose(file);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  

static void processInput(GLFWwindow *window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
	}
}

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

	Model model = {
		.meshPath = "src/models/teapot.obj"
	};
	// will set with id system later
	loadModelObj(&model);
	//dataToBuffers(&model, (Vertex*)vertices, 8, indices, 6);

	// Creating a texture with stb image
	int width, height, nrChannels;
	BMPImage imageData;
	loadBMPImage("src/textures/fox.bmp", &imageData); // loads texture data into currently bound texture
	// stbi_set_flip_vertically_on_load(true);
	//unsigned char *imageData = stbi_load("src/fox.jpg", &width, &height, &nrChannels, 0); 
	glGenTextures(1, &model.texture);
	glBindTexture(GL_TEXTURE_2D, model.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, imageData);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageData.width, imageData.height, 0, GL_BGR, GL_UNSIGNED_BYTE, imageData.data);
	glGenerateMipmap(GL_TEXTURE_2D);
	free(imageData.data);

	 //When multiplying matrices the right-most matrix is first multiplied with the vector so you should read the multiplications from right to left. It is advised to first do scaling operations, then rotations and lastly translations when combining matrices
	//If transpose is GL_FALSE, each matrix is assumed to be supplied in column major order. If transpose is GL_TRUE, each matrix is assumed to be supplied in row major order
	//local space -> model matrix -> world space -> view matrix -> view space -> projection matrix -> clip space -> viewport transform -> screen space
	//TODO abstract this awful code

	unsigned int projMatLoc = glGetUniformLocation(shaderProgram, "projection");
	unsigned int viewMatLoc = glGetUniformLocation(shaderProgram, "view");
	unsigned int modelMatLoc = glGetUniformLocation(shaderProgram, "model");

	float modelMat[16] = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
	};

	float viewMat[16] = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
	};

	float projectionMat[16] = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
	};
	
	float rot[] = {0.0, 0.0, 0.0};
	float trans[] = {0.0, 0.0, -10.0};

	glm_rotate((float (*)[4])modelMat, 0.0f, rot);
	glm_translate((float (*)[4])viewMat, trans);
	glm_perspective(0.780f, (float)(880/600), 0.1f, 100.0f, (float(*)[4])projectionMat);

	glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, (float*)projectionMat);
	glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, (float*)viewMat);
	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, (float*)modelMat);

	while(!glfwWindowShouldClose(window)){
		processInput(window);
		//lookat(window);
		// rendering commands here
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		// bind texture
		//TODO make initalizer nullify fields
		/*
		if(model.texture!=NULL){
			//glBindTexture(GL_TEXTURE_2D, model.texture);
		}
		*/
		// render container
		glUseProgram(shaderProgram);
        glBindVertexArray(model.VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
		//TODO remove wireframe after testing
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElements(GL_TRIANGLES, model.indicesNo, GL_UNSIGNED_INT, 0);
//		glBindVertexArray(0);
		// draw buffer swap
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// deallocate vao,vbo,ebo
	glfwTerminate();
	return 0;
}
