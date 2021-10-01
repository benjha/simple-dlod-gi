#ifndef __shader_object
#define __shader_object

#define SHADER_OBJECT_MAX_VARIABLES 32

class ShaderObject{
public:
	ShaderObject(char *vertex, char *fragment);
	~ShaderObject();
	void setUniformi(char *name, int value);
	void setUniformf(char *name, float value);
	void setUniformiv(char *name, int *value, int len);
	void setUniformfv(char *name, float *value, int len);
	void activate();
	static void deactivate();

	unsigned int shader_id;
	int num_vars;
	char *variable_names[SHADER_OBJECT_MAX_VARIABLES];
	int variable_ids[SHADER_OBJECT_MAX_VARIABLES];
	unsigned int variable_types[SHADER_OBJECT_MAX_VARIABLES];
	int variable_sizes[SHADER_OBJECT_MAX_VARIABLES];
private:
	void initShaders(char *vertex, char *fragment);
	char *loadShaderCode (char* fileName);
	void initVarInfo();
};

#endif