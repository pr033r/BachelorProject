#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 Model;
uniform mat4 MVP;

void main(){
	
	FragPos = vec3(Model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(Model))) * aNormal;  
	gl_Position = MVP * vec4(FragPos, 1.0);
	TexCoords = aTexCoords;

	// Output position of the vertex, in clip space : MVP * position
	//gl_Position =  MVP * vec4(aPos, 1);
	
}