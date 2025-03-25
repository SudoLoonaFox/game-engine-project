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

const char* MODEL_IN_FORMAT = "%d,%[^,],%[^,],%[^,]";

// will have multiple ones for different rendering types
const char* vertexShaderPath = "src/shaders/vertexShader.glsl";
const char* fragmentShaderPath = "src/shaders/fragmentShader.glsl";

float deltaTime = 0.0f;
float lastFrame = 0.0f;

typedef struct{
	char* data;
	int width;
	int height;
}BMPImage;

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

typedef struct{
	char name[20];
	float fov;
  Positon pos;
  Quaternion rot;
  LookAt lookAt;
}Camera;

#pragma pack(push, 1)
typedef struct{
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
  union{
    float vt[2];
  struct{
	  float u;
	  float v;
    };
  };
}Vertex;

typedef struct{ // vertex indices
	unsigned int v1;
	unsigned int v2;
	unsigned int v3;
}Face;
#pragma pack(pop)

/*
void eulerToQuaternion(float v[3], float q[4]){
  float cx = cos(v[0]/2);
  float sx = sin(v[0]/2);
  float cy = cos(v[1]/2);
  float sy = sin(v[1]/2);
  float cz = cos(v[2]/2);
  float sz = sin(v[2]/2);
  q[0] = sx*cy*cz - cx*sy*sz;
  q[1] = cx*sy*cz + sx*cy*sz;
  q[2] = cx*cy*sz - sx*sy*cz;
  q[3] = cx*cy*cz + sx*sy*sz;
}
*/
void rotateVec3(float v[3], float q[4]){
//  model = model.xyz + 2.0*cross(cross(model.xyz, rotation.xyz) + rotation.w*model.xyz, rotation.xyz);
  float tmp[3];
  float tmp2[3];
  glm_vec3_cross(v, q, tmp);
  glm_vec3_scale(v, q[3], tmp2);
  glm_vec3_add(tmp, tmp2, tmp);
  glm_vec3_cross(tmp, q, tmp);
  glm_vec3_scale(tmp, 2, tmp);
  glm_vec3_add(v, tmp, v);
}

Camera* initCamera(char* name, float fov){
  Camera* camera = malloc(sizeof(Camera));
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
  float s = sin(theta/2.0);
  dest[0] = axis[0] * s;
  dest[1] = axis[1] * s;
  dest[2] = axis[2] * s;
  dest[3] = cos(theta/2.0);
}

// takes in a lookat matrix and pitch roll yaw angles
void relLookCam(float* r, float* u, float* d, float* pos, float pitch, float yaw, float roll, float x, float y, float z, float transSpeed, float rotSpeed, float deltaTime, float dest[16]){
  // TODO move into general relative angles to transform lookat matrix
  float rot[4];
  float tmp_axis[3];

  glm_vec3_normalize(d);
  glm_vec3_normalize(u);
  glm_vec3_normalize(r);

  tmp_axis[0] = 1;
  tmp_axis[1] = 0;
  tmp_axis[2] = 0;

  // pitch
  axisAngleToQuat(tmp_axis, pitch * rotSpeed * deltaTime, rot);
  rotateVec3(r, rot);
  rotateVec3(d, rot);
  rotateVec3(u, rot);

  tmp_axis[0] = 0;
  tmp_axis[1] = 0;
  tmp_axis[2] = 1;

  // roll
  axisAngleToQuat(tmp_axis, roll * rotSpeed * deltaTime, rot);
  rotateVec3(d, rot);
  rotateVec3(r, rot);
  rotateVec3(u, rot);

  tmp_axis[0] = 0;
  tmp_axis[1] = 1;
  tmp_axis[2] = 0;

  // yaw
  axisAngleToQuat(tmp_axis, yaw * rotSpeed * deltaTime, rot);
  rotateVec3(u, rot);
  rotateVec3(r, rot);
  rotateVec3(d, rot);

  float trans[3] = {x, y, z};
  pos[0] += glm_dot(r, trans) * deltaTime * transSpeed;
  pos[1] += glm_dot(u, trans) * deltaTime * transSpeed;
  pos[2] += glm_dot(d, trans) * deltaTime * transSpeed;

  dest[0] = r[0];
  dest[1] = r[1];
  dest[2] = r[2];
  dest[3] = 0;
  dest[4] = u[0];
  dest[5] = u[1];
  dest[6] = u[2];
  dest[7] = 0;
  dest[8] = d[0];
  dest[9] = d[1];
  dest[10] = d[2];
  dest[11] = 0;
  dest[12] = 0;
  dest[13] = 0;
  dest[14] = 0;
  dest[15] = 1;

  float p[3];
  p[0] = pos[0] * -1.0;
  p[1] = pos[1] * -1.0;
  p[2] = pos[2] * -1.0;
  glm_translate((float (*)[4])dest, p);
}

void calcVertexNormals(Vertex* vertices, unsigned int verticesLen, unsigned int* indices, unsigned int indicesLen){
  // for each vertex find adjacent faces and calculate their normals
  float* faceNormals = malloc(indicesLen/3 * sizeof(float)*3);
  float* faceSurfaceArea = malloc(indicesLen/3 * sizeof(float));
  for(int i = 0; i < indicesLen/3; i++){
    /*
    * 
    *  face a normal
    *  face abc normal is cross 
    *     C
    *    / \
    *   /   \
    *  /     \
    * A-------B 
    * 
    *  face normal is bc x ba
    *  bc is c - b
    *  ba is a - b
    *  n is normal
    */

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
    glm_vec3_cross(ab, ac, n);
    glm_vec3_normalize(n);
  }
  // for every vertex check all faces for match
  // TODO optimize this
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
}

Model* dataToBuffers(Model* model, Vertex* vertices, unsigned int verticesLen, unsigned int* indices){
  printf("Started dataToBuffers\n");
  glGenVertexArrays(1, &model->VAO);
  glGenBuffers(1, &model->VBO);
  glGenBuffers(1, &model->EBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(model->VAO);

  glBindBuffer(GL_ARRAY_BUFFER, model->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*verticesLen, vertices, GL_STATIC_DRAW);

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

void loadModelObj(Model* model){
	const int DEFAULT_SIZE = 5000000;
	FILE* file = fopen(model->meshPath, "r");
	if(file == NULL){
		return;
	}
	// TODO Add reallocation
	unsigned int verticesLen = 0;
	unsigned int textureCoordinatesLen = 0;
	unsigned int vertexNormalsLen = 0;
	unsigned int faceIndex = 0;
	Vertex* vertices = malloc(sizeof(Vertex)*DEFAULT_SIZE);
  // can do this without malloc
	float* textureCoordinates = malloc(sizeof(float)*2*DEFAULT_SIZE);
	float* vertexNormals = malloc(sizeof(float)*2*DEFAULT_SIZE);
	unsigned int* indices = malloc(sizeof(unsigned int)*DEFAULT_SIZE*3);
	// IMPORTANT: obj indexing starts at 1
	char line[128];
	while(fgets(line, 128, file)){
		if(line[0] == 'v' && line[1] == ' '){
			sscanf(line, "v %f %f %f\n", &vertices[verticesLen].x, &vertices[verticesLen].y, &vertices[verticesLen].z);
			verticesLen++;
		}
		else if(line[0] == 'v' && line[1] == 't'){
			sscanf(line, "vt %f %f\n", &textureCoordinates[2*textureCoordinatesLen], &textureCoordinates[2*textureCoordinatesLen+1]);
			textureCoordinatesLen++;
		}
		else if(line[0] == 'v' && line[1] == 'n'){
			sscanf(line, "vn %f %f %f", &vertexNormals[vertexNormalsLen*3], &vertexNormals[vertexNormalsLen*3+1], &vertexNormals[vertexNormalsLen*3+2]);
			vertexNormalsLen++;
		}
		else if(line[0] == 'f' && line[1] == ' '){
			// possible formats are:
			// f v v v
			// f v/vt v/vt v/vt
			// f v/vt/vn v/vt/vn v/vt/vn
			// f v//vn v//vn v//vn
			if(3 == sscanf(line, "f %d %d %d", &indices[faceIndex*3], &indices[faceIndex*3+1], &indices[faceIndex*3+2])){
				indices[faceIndex*3]--;
				indices[faceIndex*3+1]--;
				indices[faceIndex*3+2]--;
				faceIndex++;
				continue;
			}
      unsigned int vt0;
      unsigned int vt1;
      unsigned int vt2;
      unsigned int vn0;
      unsigned int vn1;
      unsigned int vn2;

      if(9==sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &indices[faceIndex*3], &vt0, &vn0, &indices[faceIndex*3+1], &vt1, &vn1, &indices[faceIndex*3+2], &vt2, &vn2)){
				indices[faceIndex*3]--;
				indices[faceIndex*3+1]--;
				indices[faceIndex*3+2]--;
        vn0--;
        vn1--;
        vn2--;
        vt0--;
        vt1--;
        vt2--;

        memcpy(&vertices[indices[faceIndex*3+0]].vn, &vertexNormals[3*vn0], sizeof(float)*3);
        memcpy(&vertices[indices[faceIndex*3+1]].vn, &vertexNormals[3*vn1], sizeof(float)*3);
        memcpy(&vertices[indices[faceIndex*3+2]].vn, &vertexNormals[3*vn2], sizeof(float)*3);

        memcpy(&vertices[indices[faceIndex*3+0]].vt, &textureCoordinates[2*vt0], sizeof(float)*2);
        memcpy(&vertices[indices[faceIndex*3+1]].vt, &textureCoordinates[2*vt1], sizeof(float)*2);
        memcpy(&vertices[indices[faceIndex*3+2]].vt, &textureCoordinates[2*vt2], sizeof(float)*2);

				faceIndex++;
				continue;
      }
		}
	}

  unsigned int indicesLen = faceIndex*3;
	model->indicesLen = indicesLen; 

  if(!vertexNormalsLen){
    calcVertexNormals(vertices, verticesLen, indices, indicesLen);
  }

	dataToBuffers(model, vertices, verticesLen, indices);
  free(vertices);
  free(textureCoordinates);
  free(vertexNormals);
  free(indices);
	fclose(file);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height){
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
	int fd = open(path, O_RDONLY, S_IRUSR);
	struct stat sb;
  // TODO change to assert?
	if(fstat(fd, &sb) == -1){
		printf("Error loading file\n");
	}
	char* data = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	unsigned int width, height, offset, imageSize;
	if(data[0] != 'B' || data[1] != 'M'){
		// incorrect file type
		printf("Invalid image type\n");
	}
	// TODO account for endianness
	width = *(int*)&data[0x12];
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
		.meshPath = "src/models/lantern.obj"
	};
  
	loadModelObj(&model);

	// Creating a texture with stb image
	int width, height, nrChannels;
	//loadBMPImage("src/textures/fox.bmp", &imageData); // loads texture data into currently bound texture
	stbi_set_flip_vertically_on_load(true);
	unsigned char *imageData = stbi_load("src/textures/Lantern_baseColor.png", &width, &height, &nrChannels, 0); 
  assert(imageData!=NULL);
	glGenTextures(1, &model.texture);
	glBindTexture(GL_TEXTURE_2D, model.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);
  free(imageData);

	// local space -> model matrix -> world space -> view matrix -> view space -> projection matrix -> clip space -> viewport transform -> screen space

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

	glm_perspective(0.780f, (float)(880.0/600.0), 0.1f, 100.0f, (float(*)[4])projectionMat);

	unsigned int projMatLoc = glGetUniformLocation(shaderProgram, "projection");
	unsigned int viewMatLoc = glGetUniformLocation(shaderProgram, "view");
	unsigned int translationLoc = glGetUniformLocation(shaderProgram, "translation");
	unsigned int rotationLoc = glGetUniformLocation(shaderProgram, "rotation");
	unsigned int scaleLoc = glGetUniformLocation(shaderProgram, "scale");

	glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, (float*)projectionMat);
	glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, (float*)viewMat);

  Camera* camera = initCamera("Fox Cam", 0.780f);
  float cameraSpeed = 0.1;
  float cameraRotationSpeed = 100;
  camera->pos.x = 0;
  camera->pos.y = 0;
  camera->pos.z = 30;

  Spacemouse* spacemouse = initSpacemouse();

	while(!glfwWindowShouldClose(window)){
		// calculate delta time
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
    pollSpacemouse(spacemouse);

    float pitch, yaw, roll, x, y, z;
    spacemouseEulerAngles(spacemouse, &pitch, &yaw, &roll, &x, &y, &z);

    // updates camera lookat and view matrix
    relLookCam(camera->lookAt.r, camera->lookAt.u, camera->lookAt.d, camera->pos.pos, pitch, yaw, roll,
               x, y, z, cameraSpeed, cameraRotationSpeed, deltaTime, viewMat);

	  glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, (float*)viewMat);

    float rot[4];
	  float trans[] = {0.0, 0.0, -10.0};
    float axis[3] = {0, 0, 0};
    float scale[3] = {1.0, 1.0, 1.0};
    glm_normalize(axis);
    // convert rotation to quaternion
    axisAngleToQuat(axis, currentFrame*5, rot);

	  glUniform3fv(translationLoc, 1, trans);
	  glUniform4fv(rotationLoc, 1, rot);
	  glUniform3fv(scaleLoc, 1, scale);

		// rendering commands here
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// bind texture
		glBindTexture(GL_TEXTURE_2D, model.texture);
		// render container
		glUseProgram(shaderProgram);
    glBindVertexArray(model.VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    // Wireframe mode
  	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElements(GL_TRIANGLES, model.indicesLen, GL_UNSIGNED_INT, 0);
		// draw buffer swap
		glfwSwapBuffers(window);
		glfwPollEvents();
    printf("%f\n", 1.0/deltaTime);
	}
	glfwTerminate();
	return 0;
}
