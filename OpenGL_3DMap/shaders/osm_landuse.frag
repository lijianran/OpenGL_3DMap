
#version 330 core

struct Material{
		sampler2D ambientMap;
		sampler2D specularMap;

		float shininess;
		vec3 Ka;
		vec3 Kd;
		vec3 Ks;
};

struct Light{
		vec3 direction;//定向光源
//默认光源的颜色为 (1.0， 1.0， 1.0) 纯白光
		//vec3 ambient;
		//vec3 diffuse;
		//vec3 specular;
};

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main()
{
		//ambient
		vec3 ambient = material.Ka * vec3(texture2D(material.ambientMap, TexCoords));

		//diffuse
		vec3 norm = normalize(Normal);
		//vec3 lightDir = normalize(light.position - FragPos);
		vec3 lightDir = normalize(-light.direction);
		float diff = max(dot(norm, lightDir), 0.0f);
		vec3 diffuse = material.Kd * (diff * vec3(texture2D(material.ambientMap, TexCoords)));

		//specular
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
		vec3 specular = material.Ks * (spec * vec3(texture2D(material.specularMap, TexCoords)));

		//all
		vec3 result =  ambient + diffuse + specular;
		FragColor = vec4(result, 1.0f);
		//FragColor = vec4(1.0f);

}
