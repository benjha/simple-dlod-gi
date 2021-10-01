/*
  Render to vertex array

  This class implements "render to vertex array" functionality using vertex and pixel buffer objects (VBO and PBO).

  Operation:
  1. A buffer object is created
  2. The buffer object is bound to the pixel pack (destination) buffer
  3. glReadPixels is used to read from the frame buffer to the buffer object
  4. The buffer object is bound to the vertex array
  5. Vertex array pointers are set
  
  Usage:
  1. Create a floating point pbuffer
  2. Create a RenderVertexArray object for each vertex attribute you want to render to
  3. Render vertex data to pbuffer using a fragment program (could use multiple draw buffers here)
  4. Call Read() method to read data from pbuffer to vertex array
  5. Call SetPointer() method to set vertex array pointers
  6. Set any other other static vertex array attribute pointers
  6. Render geometry as usual using glDrawArrays or glDrawElements

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



  sgreen 2/2004
*/
#include <stdio.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include "../Include/renderVertexArray.h"

RenderVertexArray::RenderVertexArray(int nverts, GLint size, GLenum type) :
    m_nverts(nverts), m_size(size), m_usage(GL_STATIC_COPY), m_type(type)
{
    switch(m_type) {
        case GL_HALF_FLOAT_NV:
            m_bytes_per_component = 2; break;
        case GL_FLOAT:
            m_bytes_per_component = sizeof(float); break;
        default:
            fprintf(stderr, "Error: unsupported RenderVertexArray type\n");
            return;
    }

    // create the buffer object
    glGenBuffersARB(1, &m_buffer);
    glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, m_buffer);
    glBufferDataARB(GL_PIXEL_PACK_BUFFER_EXT, m_nverts*m_size*m_bytes_per_component, 0, m_usage); // undefined data
    glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, 0);

    // set equivalent image format
    switch(m_size) {
        case 1:
            m_format = GL_LUMINANCE; break;
        case 3:
            m_format = GL_RGB; break;
        case 4:
            m_format = GL_RGBA; break;
        default:
            fprintf(stderr, "Error: unsupported RenderVertexArray size\n");
            return;
    }
}

RenderVertexArray::~RenderVertexArray()
{
    glDeleteBuffersARB(1, &m_buffer);
}

void
RenderVertexArray::LoadData(void *data)
{
    // load data to buffer object
    glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, m_buffer);
    glBufferDataARB(GL_PIXEL_PACK_BUFFER_EXT, m_nverts*m_size*m_bytes_per_component, data, m_usage);
}

void
RenderVertexArray::Read(GLenum buffer, int w, int h)
{
    // bind buffer object to pixel pack buffer
    glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, m_buffer);
    // read from frame buffer to buffer object
    glReadBuffer(buffer);
    glReadPixels(0, 0, w, h, m_format, m_type, 0);

    glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, 0);
}

void
RenderVertexArray::SetPointer(GLuint index)
{
    // bind buffer object to vertex array 
    glBindBufferARB(GL_ARRAY_BUFFER, m_buffer);
    glVertexAttribPointerARB(index, m_size, m_type, GL_FALSE, 0, 0);

    glBindBufferARB(GL_ARRAY_BUFFER, 0);
}
