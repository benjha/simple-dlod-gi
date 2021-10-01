#ifndef __string_utils
#define __string_utils

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#define stricmp strcasecmp
#endif

class StringUtils{
public:
	static char *registerString(char *str);
	static void reset();
	static void splitDir(char *filePath, char *dir, char *file);
	static int getStyleId(char *name);
	static int parseIntList(char *data, int *array);
};

#endif