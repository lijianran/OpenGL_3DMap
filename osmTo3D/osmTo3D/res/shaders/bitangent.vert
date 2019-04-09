
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 4) in vec3 bitangent;

out VS_OUT{
		vec3 bitangent;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main(){
  gl_Position = projection * view * model * vec4(position, 1.0);
		mat3 bitangentMatrix  = mat3(transpose(inverse(view * model)));
		vs_out.bitangent = vec3(projection * normalize(vec4(bitangentMatrix * bitangent, 1.0)));
}
