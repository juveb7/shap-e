#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertUV;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform vec3 lightP;
uniform vec3 viewP;

out vec3 fragNor;
out vec3 fragP;
out vec3 lightDir;
out vec3 viewDir;
out vec3 halfDir;
out vec2 TexCoords;

void main()
{
	gl_Position = P * V * M * vertPos;
	fragNor = (V*M * vec4(vertNor, 0.0)).xyz;
	fragP = vec3(M*vertPos);
	lightDir = normalize(lightP - fragP);
	viewDir = normalize(viewP - fragP);
	halfDir = normalize(lightDir + viewDir);
	TexCoords = vertUV;
}
