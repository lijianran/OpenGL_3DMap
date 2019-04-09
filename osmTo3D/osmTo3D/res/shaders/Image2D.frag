
#version 330 core
out vec4 FragColor;

uniform sampler2D ambientMap;
in vec2 TexCoords;

void main()
{
				FragColor = texture2D(ambientMap, TexCoords);
}
