/*
  Render to vertex array class
  sgreen 2/2004

	Generic
    Attribute   Conventional Attribute       Conventional Attribute Command
    ---------   ------------------------     ------------------------------
         0      vertex position              Vertex
         1      vertex weights 0-3           WeightARB, VertexWeightEXT
         2      normal                       Normal
         3      primary color                Color
         4      secondary color              SecondaryColorEXT
         5      fog coordinate               FogCoordEXT
         6      -                            -
         7      -                            -
         8      texture coordinate set 0     MultiTexCoord(TEXTURE0, ...)
         9      texture coordinate set 1     MultiTexCoord(TEXTURE1, ...)
        10      texture coordinate set 2     MultiTexCoord(TEXTURE2, ...)
        11      texture coordinate set 3     MultiTexCoord(TEXTURE3, ...)
        12      texture coordinate set 4     MultiTexCoord(TEXTURE4, ...)
        13      texture coordinate set 5     MultiTexCoord(TEXTURE5, ...)
        14      texture coordinate set 6     MultiTexCoord(TEXTURE6, ...)
        15      texture coordinate set 7     MultiTexCoord(TEXTURE7, ...)
       8+n      texture coordinate set n     MultiTexCoord(TEXTURE0+n, ...)


*/

#ifndef RENDERVERTEXARRAY_H
#define RENDERVERTEXARRAY_H

#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>

enum {
	VERTEX=0,
	NORMAL=2,
	TEXTURE0=8,
	TEXTURE1=9
};

class RenderVertexArray {
public:
  RenderVertexArray(int nverts, GLint size, GLenum type = GL_FLOAT);
  ~RenderVertexArray();

  void LoadData(void *data);                // load vertex data from memory
  void Read(GLenum buffer, int w, int h);   // read vertex data from frame buffer
  void SetPointer(GLuint index);

private:
    GLenum m_usage;     // vbo usage flag
    GLuint m_buffer;
    GLuint m_index;
    GLuint m_nverts;
    GLint m_size;       // size of attribute       
    GLenum m_format;    // readpixels image format
    GLenum m_type;      // FLOAT or HALF_FLOAT
    int m_bytes_per_component;
};

#endif