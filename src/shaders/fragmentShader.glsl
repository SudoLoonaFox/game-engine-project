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

void main(){
  // TODO set up real lights
  vec3 norm = normalize(vertexNormal);
  vec3 vecLight = vec3(1.2, 1.0, 2.0);
  vec3 lightDir = normalize(vecLight - fragPos);
  float diff = max(dot(norm, vecLight), 0.0);
	/*
	// ambient
	vec3 ambient = lightColor * material.ambient;

	// diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightPos), 0.0);
	vec3 diffuse = lightColor * (diff * material.diffuse);

	// specular
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = lightColor * spec * material.specular;

	vec3 result = ambient + diffuse + specular;

*/
//	FragColor = texture(texture1, texCoord);
	//FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
	//FragColor = normalize(vec4(1.0f, 0.5f, 0.2f, 1.0f));
  // basic hue shift
	vec4 baseColor = normalize(vec4(1.0f, 0.5f, 0.2f, 1.0f));
  //FragColor = baseColor * vec4(vec3(diff), 1.0);
  FragColor = vec4(vertexNormal, 1.0);
  //FragColor = normalize(baseColor + vec4(0.8*diff, 0.0, -0.8*diff, 1.0f));
	FragColor = vec4(vec3(1.0f, 0.5f, 0.2f)*diff, 1.0f);

	//FragColor = normalize(vec4(1.0f, 0.5f, 0.2f, 1.0f));
//	FragColor = vec4(result, 1.0);
}
