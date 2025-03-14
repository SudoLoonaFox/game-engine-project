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
#include <cglm/vec3.h>

#include <math.h>

#include "input.h"

// Not in use currently
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// TODO make functions structure independent where possible
//
// TODO add quaternions and controller math
//
// TODO Make Camera controls
//
// TODO Add delaunay triangulation

// Specified in models.csv id,name,meshpath,texturepath
const char* MODEL_IN_FORMAT = "%d,%[^,],%[^,],%[^,]";

// will have multiple ones for different rendering types
const char* vertexShaderPath = "src/shaders/vertexShader.glsl";
const char* fragmentShaderPath = "src/shaders/fragmentShader.glsl";

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// add info to models.csv for default scaling, rot, pos etc
// entities will have additional information like current pos, scaling, rot, etc in world space as well as movement vectors and the like
typedef struct{
	char* data;
	int width;
	int height;
}BMPImage;

//TODO add camera movement smoothing, interpolation, look at target, follow mouse etc
// Do I add stuff for tracking in the struct like a target?
// tracking speeds? limits? 
// Homogeneous transformation matrix?
// Quaternions?
// Need to get a homogeneous transformation matrix for the view math but quaternions would be best for storing angles
// make a position struct that is transform, scale, and rot?

typedef union{
  float pos[3];
  struct{
    float x;
    float y;
    float z;
  };
}Positon;

typedef union{
  float q[4];
  struct{
    float x;
    float y;
    float z;
    float w;
  };
}Quaternion;

typedef struct{
  float r[3];
  float u[3];
  float d[3];
}LookAt;

typedef struct{
	unsigned int id;
	char name[30];
	char meshPath[30];
	char texturePath[30];
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	unsigned int indicesLen;
	unsigned int texture;
  Positon pos;
	Quaternion rot;
  float scale;
  float modelMat[16];
}Model;

// do we attach the camera to an object or object to the camera?
// do we need a kill cam or stuff like that? changing scene with one active would caues a hanging reference
typedef struct{
	char name[20];
	float fov;
  Positon pos;
  Quaternion rot;
  LookAt lookAt;
}Camera;

// TODO  decide if xyz, rgb, etc should be a vec type
// float xyz and access by vertex.xyz[0] or vertex.x
#pragma pack(push, 1)
typedef struct {
  union{
    float pos[3];
    struct{
      float x;
      float y;
      float z;
    };
  };
	float r;
	float g;
	float b;
  union{
    float vn[3];
    struct{
      float nx; // vertexNormals
      float ny;
      float nz;
    };
  };
	float u;
	float v;
}Vertex;

typedef struct { // vertex indices
	unsigned int v1;
	unsigned int v2;
	unsigned int v3;
}Face;
#pragma pack(pop)

// TODO create lookat init for other stuff

Camera* initCamera(char* name, float fov){
  Camera* camera = malloc(sizeof(Camera));
  // checkstrlen
  strcpy(camera->name, name);
  camera->fov = fov;

  camera->lookAt.d[0] = 0;
  camera->lookAt.d[1] = 0;
  camera->lookAt.d[2] = 1;

  camera->lookAt.u[0] = 0;
  camera->lookAt.u[1] = 1;
  camera->lookAt.u[2] = 0;

  camera->lookAt.r[0] = 1;
  camera->lookAt.r[1] = 0;
  camera->lookAt.r[2] = 0;

  return camera;
}

void axisAngleToQuat(float axis[3], float theta, float dest[4]){
  float s = sin(theta/2);
  dest[0] = axis[0] * s;
  dest[1] = axis[1] * s;
  dest[2] = axis[2] * s;
  dest[3] = cos(theta/2);
}

void calcVertexNormals(Vertex* vertices, unsigned int verticesLen, unsigned int* indices, unsigned int indicesLen){
  // for each vertex find adjacent faces and calculate their normals
  // trying with dotproduct. may need to convert cos theta to theta
  // TODO rewrite as compute shader
  printf("Started calcVertexNormals\n");
  float* faceNormals = malloc(indicesLen/3 * sizeof(float)*3);
  float* faceSurfaceArea = malloc(indicesLen/3 * sizeof(float));
  for(int i = 0; i < indicesLen/3; i++){
    // face a normal
    // face abc normal is cross 
    //    C
    //   / \
    //  /   \
    // /     \
    //A-------B 
    //
    // face normal is bc x ba
    // bc is c - b
    // ba is a - b
    // n is normal

    float* n = &faceNormals[3*i];
    float ab[3];
    float ac[3];
    float* a = vertices[indices[i*3+0]].pos;
    float* b = vertices[indices[i*3+1]].pos;
    float* c = vertices[indices[i*3+2]].pos;
    glm_vec3_sub(b, a, ab);
    glm_vec3_sub(c, a, ac);

    glm_vec3_cross(ab, ac, n);
    
    // area of face abc is 0.5*||AB X AC||
    faceSurfaceArea[i] = n[0]*n[0] + n[1]*n[1] + n[2]*n[2];
    faceSurfaceArea[i] = sqrtf(faceSurfaceArea[i]) * 0.5;
    //printf("faceSurfaceArea: %f\n",faceSurfaceArea[i]);

    //glm_vec3_normalize(ab);
    //glm_vec3_normalize(ac);
    glm_vec3_cross(ab, ac, n);
    glm_vec3_normalize(n);
    /*
    n[0] = 0;
    n[1] = 0;
    n[2] = 1;
    */
    //printf("faceNormal:\t%f\t%f\t%f\n",n[0],n[1],n[2]);
  }
  // for every vertex check all faces for match
  for(int v = 0; v < verticesLen; v++){
    float n[3] = {0, 0, 0};
    // check every face for vertex match
    for(int i = 0; i < indicesLen; i++){
      if(indices[i] == v){
        // trying finding angle within face b around shared vert
        float temp[3];
        // angle for triangle abc with shared b is ba, bc
        float temp2[3];
        glm_vec3_sub(vertices[v].pos, vertices[indices[3*(i/3)+(i+1)%3]].pos, temp);
        glm_vec3_sub(vertices[v].pos, vertices[indices[3*(i/3)+(i+2)%3]].pos, temp2);
        float angle = glm_vec3_angle(temp, temp2);
        glm_vec3_scale(&faceNormals[3*(i/3)], faceSurfaceArea[i/3],temp);
        glm_vec3_scale(temp, angle, temp);
        glm_vec3_add(n, temp, n);
        continue;
      }
    }
    glm_vec3_normalize(n);
    glm_vec3_copy(n, vertices[v].vn);
  }
  free(faceNormals);
  free(faceSurfaceArea);
  printf("Finished calcVertexNormals\n");
}


//TODO add texture here? or elsewhere
Model* dataToBuffers(Model* model, Vertex* vertices, unsigned int verticesLen, unsigned int* indices){
  printf("Started dataToBuffers\n");
  glGenVertexArrays(1, &model->VAO);
  glGenBuffers(1, &model->VBO);
  glGenBuffers(1, &model->EBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(model->VAO);


  printf("Test1\n\n");
  glBindBuffer(GL_ARRAY_BUFFER, model->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*verticesLen, vertices, GL_STATIC_DRAW);
  printf("Test2\n");

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*model->indicesLen, indices, GL_STATIC_DRAW);
	int stride = sizeof(Vertex);
	// position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, stride, (void*)0);
  glEnableVertexAttribArray(0);
	// color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// vertex normal
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, stride, (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	// texture coordinate
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_TRUE, stride, (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);

  // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
  // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
  glBindVertexArray(0);
  printf("Finished dataToBuffers\n");
	return model;
}
	
// TODO modify this to be more useful
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
		//printf("id: %d, name: %s, path: %s\n", models[i].id, models[i].name, models[i].meshPath);
	}
	return models;
}
void getModelFromIndex(int id, Model* model);

// TODO Finish loading code
// TODO Change to generic type stuff 
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
		//printf("Line: %s\n", line);

		if(line[0] == 'v' && line[1] == ' '){
			sscanf(line, "v %f %f %f\n", &vertices[verticesLen].x, &vertices[verticesLen].y, &vertices[verticesLen].z);
			verticesLen++;
			//printf("Vertex Added NO %i\n", verticesLen);
		}
		else if(line[0] == 'v' && line[1] == 't'){
			sscanf(line, "vt %f %f\n", &textureCoordinates[2*textureCoordinatesLen], &textureCoordinates[2*textureCoordinatesLen+1]);
			//printf("Texture\n");
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
				//printf("f d d d");
				continue;
			}
			int vt[3];
			int vn[3];

      // TODO Finish this
      int b = 0;
      int* f = &b;
      if(9==sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &indices[faceIndex*3], f, f, &indices[faceIndex*3+1], f, f, &indices[faceIndex*3+2], f, f)){
				indices[faceIndex*3]--;
				indices[faceIndex*3+1]--;
				indices[faceIndex*3+2]--;
				faceIndex++;
				//printf("f d d d");
				continue;
      }
			//TODO Finish this
      /*
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
      */
				//printf("f d/d/d d/d/d d/d/d");
		}
	}
  unsigned int indicesLen = faceIndex*3;
	model->indicesLen = indicesLen; 

  /*
	// TODO calculate vertex normals
  DCEL* dcel;
	dcel = dataToHalfEdge(vertices, verticesLen, indices, model->indicesNo);
  free(dcel);
  */
  calcVertexNormals(vertices, verticesLen, indices, indicesLen);
	dataToBuffers(model, vertices, verticesLen, indices);
	// TODO all that freeing stuff
	fclose(file);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}  

static void processInput(GLFWwindow *window){
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
    glfwSetWindowShouldClose(window, true);
  }
  /*
  const float cameraSpeed = 0.05f; // adjust accordingly
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){

  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
  */
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
	// TODO account for endianness
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

  /*
	Camera camera = {
		.name = "Fox Cam",
		.fov = 0.7f // around the fov for a 50mm lens in radians
	};
  */

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

	// Move the camera

	glm_translate((float (*)[4])viewMat, trans);
	glm_perspective(0.780f, (float)(880.0/600.0), 0.1f, 100.0f, (float(*)[4])projectionMat);

	unsigned int projMatLoc = glGetUniformLocation(shaderProgram, "projection");
	unsigned int viewMatLoc = glGetUniformLocation(shaderProgram, "view");
	unsigned int translationLoc = glGetUniformLocation(shaderProgram, "translation");
	unsigned int rotationLoc = glGetUniformLocation(shaderProgram, "rotation");
	unsigned int scaleLoc = glGetUniformLocation(shaderProgram, "scale");

	glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, (float*)projectionMat);
	glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, (float*)viewMat);

  Camera* camera = initCamera("Fox Cam", 0.780f);
  camera->pos.x = 0;
  camera->pos.y = 0;
  camera->pos.z = 30;
  Spacemouse* spacemouse = initSpacemouse();

	while(!glfwWindowShouldClose(window)){
		// calculate delta time
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

    // TODO convert camera view to quaternion plus position?
    // move camera
    pollSpacemouse(spacemouse);
    float cameraSpeed = 50;

    
    camera->pos.x = camera->pos.x + (spacemouse->x / 350.0) * deltaTime * cameraSpeed;
    camera->pos.y = camera->pos.y - (spacemouse->z / 350.0) * deltaTime * cameraSpeed;
    camera->pos.z = camera->pos.z + (spacemouse->y / 350.0) * deltaTime * cameraSpeed;
    
    

    viewMat[0] = camera->lookAt.r[0];
    viewMat[1] = camera->lookAt.r[1];
    viewMat[2] = camera->lookAt.r[2];
    viewMat[3] = 0;
    viewMat[4] = camera->lookAt.u[0];
    viewMat[5] = camera->lookAt.u[1];
    viewMat[6] = camera->lookAt.u[2];
    viewMat[7] = 0;
    viewMat[8] = camera->lookAt.d[0];
    viewMat[9] = camera->lookAt.d[1];
    viewMat[10] = camera->lookAt.d[2];
    viewMat[11] = 0;
    viewMat[12] = 0;
    viewMat[13] = 0;
    viewMat[14] = 0;
    viewMat[15] = 1;

    float p[3];
    p[0] = camera->pos.pos[0] * -1.0;
    p[1] = camera->pos.pos[1] * -1.0;
    p[2] = camera->pos.pos[2] * -1.0;
    glm_translate((float (*)[4])viewMat, p);
    

	  glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, (float*)viewMat);

	  float modelMat[16];
	  float trans[] = {3.0, 0.0, -10.0};
	  //float trans[] = {5.0, 0.0, 0.0};
    float axis[3] = {0, 1, 0};
    //float axis[3] = {0.707107, 0.707107, 0};
    float scale[3] = {1.0, 1.0, 1.0};
    // convert rotation to quaternion
    float rot[4];
    axisAngleToQuat(axis, currentFrame*5, rot);


	  glUniform3fv(translationLoc, 1, trans);
	  glUniform4fv(rotationLoc, 1, rot);
	  glUniform3fv(scaleLoc, 1, scale);
		processInput(window);
		// rendering commands here
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
  	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElements(GL_TRIANGLES, model.indicesLen, GL_UNSIGNED_INT, 0);
		// draw buffer swap
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// deallocate vao,vbo,ebo
	glfwTerminate();
	return 0;
}
