
#version 330 core
out vec4 FragColor;

in VS_OUT {
				vec2 TexCoords;
				vec3 TangentLightPos;
				vec3 TangentViewPos;
				vec3 TangentFragPos;
} fs_in;

//struct LightValues{
//		float ambientStrength;
//		float diffuseStrength;
//		float specularStrength;
//};
struct LightValue{
		float ambient;
		float diffuse;
		float specular;
};

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform LightValue light;

void main()
{
				// Obtain normal from normal map in range [0,1]
				vec3 normal = texture2D(normalMap, fs_in.TexCoords).rgb;
				// Transform normal vector to range [-1,1]
				normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space

				// Get diffuse color
				vec4 color = texture2D(diffuseMap, fs_in.TexCoords);
				if(color.a < 0.1)
						discard;

				// Ambient
				vec4 ambient = color * light.ambient;
				// Diffuse
				vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
				float diff = max(dot(lightDir, normal), 0.0);
				vec4 diffuse = diff * color * light.diffuse;
				// Specular
				vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
				vec3 halfwayDir = normalize(lightDir + viewDir);
				float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
				vec4 specular = spec * color * light.specular;

//				float shadow = shadows ? ShadowCalculation(fs_in.TangentFragPos) : 0.0;
				//float shadow = ShadowCalculation(fs_in.TangentFragPos);
//				vec4 lighting = ambient + (1.0-shadow) * (diffuse + specular);

//				FragColor = lighting;
				FragColor = vec4(ambient + diffuse + specular);
			//	FragColor = vec4(vec3(ShadowCalculation(fs_in.TangentFragPos)/far_plane), 1.0f);
}
