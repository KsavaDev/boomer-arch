#ifndef SHADER_H
#define SHADER_H

#include <GL/gl.h>
typedef struct
{
  char *path;
  char *content;
} Shader;

Shader shader_read(char *file);
GLuint shader_new(Shader shader, GLenum kind);
GLuint shader_new_program(Shader vert, Shader frag);
void shader_reload(Shader *shader);

#endif // !SHADER_H

