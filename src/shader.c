#define GL_GLEXT_PROTOTYPES
#include "shader.h"
#include <GL/glext.h>
#include "file_io.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Shader shader_read(char *file)
{
  Shader result;
  result.path = strcat("src/", file);
  result.content = read_file(file);
  return result;
}

GLuint shader_new(Shader shader, GLenum kind)
{
  GLuint result = glCreateShader(kind);
  char *shader_array[] = { shader.content };
  glShaderSource(result, 1, shader_array, NULL);
  glCompileShader(result);
  
  GLuint success;
  char info_log[512];

  glGetShaderiv(result, GL_COMPILE_STATUS, &success);
  if (success == 0)
  {
    glGetShaderInfoLog(result, 512, NULL, info_log);
    printf("------------------------------\n");
    printf("Error during shader compilation: %s. Log:\n", shader.path);
    printf("%s\n", info_log);
    printf("------------------------------\n");
  }

  return result;
}

GLuint shader_new_program(Shader vert, Shader frag)
{
  GLuint result = glCreateProgram();

  GLuint vertex_shader = shader_new(vert, GL_VERTEX_SHADER);
  GLuint fragment_shader = shader_new(frag, GL_FRAGMENT_SHADER);

  glAttachShader(result, vertex_shader);
  glAttachShader(result, fragment_shader);

  glLinkProgram(result);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  GLuint success;
  char info_log[512];

  glGetProgramiv(result, GL_LINK_STATUS, &success);
  if (success == 0)
  {
    glGetProgramInfoLog(result, 512, NULL, info_log);
    printf("%s\n", info_log);
  }

  glUseProgram(result);

  return result;
}

void shader_reload(Shader *shader)
{
  shader->content = read_file(shader->path);
}
