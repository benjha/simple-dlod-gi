#include "../Include/ShaderObject.h"
#include "../Include/StringUtils.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>


ShaderObject::ShaderObject(char *vertex, char *fragment):shader_id(0),num_vars(0){
	initShaders(vertex, fragment);
}

ShaderObject::~ShaderObject(){
	if(shader_id) glDeleteShader (shader_id);
}

char *ShaderObject::loadShaderCode (char* filename){
	FILE *shFile;
	
	unsigned long len, readLen;
	char* fileData;

    struct stat f_status;
	
	if(stat(filename, &f_status)) return NULL;
	len = f_status.st_size;

	shFile = fopen(filename, "r");
	if(shFile == NULL) return NULL;

	fileData = new char[len+1];

	memset(fileData, ' ', len+1);
	readLen = fread(fileData, 1, len, shFile);
	fileData[len]=0;

	fclose(shFile);

	return fileData;
}

void ShaderObject::initShaders(char *vertex, char *fragment){
	int progLinkSuccess;
	const char *Shader;
	GLuint vertexHandler, fragmentHandler;
	GLuint programObject;
	shader_id = 0;

	if ((Shader = (const char *)loadShaderCode (vertex))==NULL){
		fprintf(stderr, "Unable to Load Vertex Shader\n");
		exit (1);
	}
	vertexHandler = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource 	(vertexHandler  , 1, &Shader, NULL);
	if (Shader) delete [] Shader, Shader = NULL;
		
	if ((Shader = (const char *)loadShaderCode (fragment))==NULL){
		fprintf(stderr, "Unable to Load  Shader\n");
		exit (1);
	}
	fragmentHandler = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource	(fragmentHandler  , 1, &Shader, NULL);
	if (Shader) delete [] Shader, Shader = NULL;

	glCompileShader	(vertexHandler);
	glCompileShader (fragmentHandler);

	programObject = glCreateProgram ();
	glAttachShader (programObject, vertexHandler);
	glAttachShader (programObject, fragmentHandler);
	
	glLinkProgram (programObject);

	glDeleteShader(vertexHandler);
	glDeleteShader(fragmentHandler);

    glGetProgramiv (programObject, GL_LINK_STATUS, &progLinkSuccess);
	
	if (!progLinkSuccess) {

		glGetProgramiv (programObject, GL_INFO_LOG_LENGTH, &progLinkSuccess);
		Shader = new const char [progLinkSuccess];
		glGetProgramInfoLog (programObject, progLinkSuccess, NULL, (char*)Shader);
		fprintf (stdout, "\n%s\n", Shader);
		fprintf(stderr, "Shader could not be linked\n");
		delete [] Shader;
		Shader = NULL;
        exit(1);
	}

	shader_id = programObject;
	glUseProgram (shader_id);
	num_vars=0;
	initVarInfo();
}

void ShaderObject::initVarInfo(){
	char *buffer;
	int len, bufLen;
	if(!shader_id) return;

	num_vars = 0;
	glGetProgramiv(shader_id, GL_ACTIVE_UNIFORMS, &num_vars);
	glGetProgramiv(shader_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &bufLen);
	buffer = new char[bufLen+1];

	for(unsigned int i=0; i<(unsigned int)num_vars; ++i){
		variable_ids[i] = i;
		glGetActiveUniform (shader_id, i, bufLen, &len, &variable_sizes[i], &variable_types[i], buffer);
		buffer[len+1]=0;
		variable_names[i] = StringUtils::registerString(buffer);
	}
	delete buffer;
}

void ShaderObject::setUniformi(char *name, int value){
	unsigned int i;
	for(i=0; i<(unsigned int)num_vars; ++i){
		if(!stricmp(name, variable_names[i])){
			glUniform1i 	(variable_ids[i], value);
			break;
		}
	}
//	i = glGetUniformLocationARB (shader_id, name);
//	glUniform1iARB(i, value);
}

void ShaderObject::setUniformf(char *name, float value){
	unsigned int i;
	for(i=0; i<(unsigned int)num_vars; ++i){
		if(!stricmp(name, variable_names[i])){
			glUniform1f	(variable_ids[i], value);
			break;
		}
	}
//	i = glGetUniformLocationARB (shader_id, name);
//	glUniform1fARB(i, value);
}

void ShaderObject::setUniformiv(char *name, int *value, int len){
	unsigned int i;
	for(i=0; i<(unsigned int)num_vars; ++i){
		if(!stricmp(name, variable_names[i])){
			switch(variable_sizes[i]){
				case 1:
					glUniform1iv (variable_ids[i], 1, value);
					break;
				case 2:
					glUniform2iv (variable_ids[i], 1, value);
					break;
				case 3:
					glUniform3iv (variable_ids[i], 1, value);
					break;
				case 4:
					glUniform4iv (variable_ids[i], 1, value);
					break;
			}
			break;
		}
	}
//	i = glGetUniformLocationARB (shader_id, name);
//	glGetActiveUniformARB(shader_id, i, bufLen, &len, &variable_sizes[i], &variable_types[i], buffer);
//	glUniform1ivARB(i, value);
}

void ShaderObject::setUniformfv(char *name, float *value, int len){
	
	for(int i=0; i<num_vars; ++i){
		if(!stricmp(name, variable_names[i])){
			switch(len){
				case 1:
					glUniform1fv  (variable_ids[i], 1, value);
					break;
				case 2:
					glUniform2fv  (variable_ids[i], 1, value);
					break;
				case 3:
					glUniform3fv  (variable_ids[i], 1, value);
					break;
				case 4:
					glUniform4fv  (variable_ids[i], 1, value);
					break;
			}
			break;
		}
	}
	
}

void ShaderObject::activate(){
	glUseProgram (shader_id);
}

void ShaderObject::deactivate(){
	glUseProgram (0);
}