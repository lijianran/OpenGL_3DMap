
#version 330 core

struct Material{
  sampler2D ambientMap;
  sampler2D diffuseMap;

  vec3 Ka;
  vec3 Kd;
  vec3 Ks;
  float shininess;
};

struct Light{
  vec3 direction;


//		float constant;
//		float linear;
//		float quadratic;
};

out vec4 color;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

uniform bool isOpenLighting;


void main(){
  //Ambient
  vec3 ambient = material.Ka * vec3(texture2D(material.ambientMap, TexCoords));

  if(isOpenLighting){
    //Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = material.Kd * diff * vec3(texture2D(material.diffuseMap, TexCoords));

    //specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    vec3 specular = material.Ks * spec * vec3(texture2D(material.diffuseMap, TexCoords));
    vec3 result = (ambient + diffuse + specular);
    color = vec4(result, 1.0f);
  }else{
    color = vec4(ambient, 1.0f);
  }
}
