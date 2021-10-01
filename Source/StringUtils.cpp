#include "../Include/StringUtils.h"
#include <string.h>
#include <stdlib.h>

#define STRING_UTILS_BUFFER_SIZE 65536
char    string_utils_buffer[STRING_UTILS_BUFFER_SIZE];
int     string_utils_buffer_ptr=0;

#define CROWD_PARSER_MAX_STYLES 256
char *style_names[CROWD_PARSER_MAX_STYLES];
int   num_styles=0;

char *StringUtils::registerString(char *str){
	int len;
	char *retVal;

	len = (int) strlen(str);
	strncpy(&string_utils_buffer[string_utils_buffer_ptr], str, len+1);
	retVal = &string_utils_buffer[string_utils_buffer_ptr];
	string_utils_buffer_ptr += len+1;

	return retVal;
}

void StringUtils::splitDir(char *filePath, char *dir, char *file){
	int bkslPtr;
	char *tmpPtr;
	int len;

	len = strlen(filePath);
	memcpy(dir, filePath, len+1);
	while((tmpPtr=strchr(dir, '/')) != NULL){
		dir[(int)(tmpPtr-dir)] = '\\';
	}

	bkslPtr = -1;
	tmpPtr = strrchr(dir, '\\');
	if(tmpPtr) bkslPtr = (int)(tmpPtr-dir+1);
	if(bkslPtr>=0){
		dir[bkslPtr-1] = 0;
		memcpy(file, &dir[bkslPtr], len-bkslPtr+1);
	} else {
		memcpy(file, filePath, len+1);
		dir[0]=0;
	}

}

int StringUtils::getStyleId(char *name){
	int i;

	for(i=0; i<num_styles; ++i){
		if(!stricmp(name, style_names[i])){
			return i;
		}
	}
	style_names[num_styles++] = StringUtils::registerString(name);
	return num_styles-1;
}

void StringUtils::reset(){
	string_utils_buffer_ptr = 0;
	num_styles = 0;
}

int StringUtils::parseIntList(char *data, int *array){
	char buffer[256];
	int dataLen;
	int i, arrayLen;

	dataLen = strlen(data);
	arrayLen = 0;
	memcpy(buffer, data, dataLen);
	for(i=0; i<dataLen; ++i){
		if(buffer[i]==','){
			buffer[i] = 0;
		}
	}
	array[arrayLen++] = atoi(buffer);
	for(i=1; i<dataLen; ++i){
		if(buffer[i-1]==0){
			array[arrayLen++] = atoi(&buffer[i]);
		}
	}
	return arrayLen;
}

