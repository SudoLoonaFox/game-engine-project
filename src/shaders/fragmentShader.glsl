#version 330 core

in vec3 outputColor;
in vec3 vertexNormal;
in vec2 texCoord;
out vec4 FragColor;

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
	FragColor = texture(texture1, texCoord);
	FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
//	FragColor = vec4(result, 1.0);
}
