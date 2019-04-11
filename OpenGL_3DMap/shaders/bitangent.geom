
#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
				vec3 bitangent;
} gs_in[];

const float MAGNITUDE = 0.1;

void GenerateLine(int index)
{
				gl_Position = gl_in[index].gl_Position;
				EmitVertex();
				gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].bitangent, 0.0) * MAGNITUDE;
				EmitVertex();
				EndPrimitive();
}

void main()
{
				GenerateLine(0); // 第一个顶点法线
				GenerateLine(1); // 第二个顶点法线
				GenerateLine(2); // 第三个顶点法线
}
