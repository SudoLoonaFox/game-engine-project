#version 330 core

in vec3 outputColor;
in vec3 vertexNormal;
in vec2 texCoord;
out vec4 FragColor;
in vec3 fragPos;

/*
struct Material{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

uniform Material material;

*/
uniform sampler2D texture1;

vec3 lightColor = vec3(1, 1, 1);
float ambientStrength = 0.2;

void main(){
  // TODO set up real lights
	// normalize lighting and have intensity value too
  vec3 norm = normalize(vertexNormal);
  vec3 vecLight = vec3(1.2, 1.0, 2.0);
  vec3 lightDir = normalize(vecLight - fragPos);
  float diff = max(dot(norm, vecLight), 0.0);

  //FragColor = vec4(texture(texture1, texCoord).xyz * diff, 1.0);
  FragColor = vec4(texture(texture1, texCoord).xyz * max(diff, ambientStrength), 1.0);
  //FragColor = vec4(1, 0, 1, 1);
  //FragColor = vec4(FragColor.xyz + max(texture(texture1, texCoord).xyz, texture(texture1, texCoord).xyz*ambient.xyz), 1.0f);

	//FragColor = normalize(vec4(1.0f, 0.5f, 0.2f, 1.0f));
//	FragColor = vec4(result, 1.0);
}
