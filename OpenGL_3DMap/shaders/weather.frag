
#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D diffuse;

void main()
{
			vec4 color = texture2D(diffuse, TexCoord);
			if(color.a < 0.1 || color.r > 0.7)
						discard;
				FragColor = color;
}
