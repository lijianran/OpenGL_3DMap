
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 3) in vec3 tangent;

out VS_OUT{
		vec3 tangent;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main(){
  gl_Position = projection * view * model * vec4(position, 1.0);
		mat3 tangentMatrix  = mat3(transpose(inverse(view * model)));
		vs_out.tangent = vec3(projection * normalize(vec4(tangentMatrix * tangent, 1.0)));
}
