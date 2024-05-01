#version 330 core 
in vec3 fragNor;
in vec3 lightDir;
in vec3 viewDir;
in vec3 halfDir;

out vec4 color;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;

void main()
{
	vec3 normal = normalize(fragNor);
	vec3 Ncolor = 0.5*normal + 0.5;
	vec3 lightColor = vec3(1, 1, 1);

	if (MatAmb.r > 0.0)
	{
		float diff = max(dot(normal, lightDir), 0.0);
		float spec = pow(max(dot(viewDir, halfDir), 0.0), MatShine);
		color = vec4(MatAmb*lightColor + diff*MatDif*lightColor+ spec*MatSpec*lightColor, 1.0);
	}
	else
	{
		color = vec4(Ncolor, 1.0);
	}

}
