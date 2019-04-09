
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 2) in vec3 normal;

out VS_OUT{
  vec3 normal;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main(){
  gl_Position = projection * view * model * vec4(position, 1.0);
  mat3 normalMatrix  = mat3(transpose(inverse(view * model)));
  vs_out.normal = vec3(projection * normalize(vec4(normalMatrix * normal, 1.0)));
}
