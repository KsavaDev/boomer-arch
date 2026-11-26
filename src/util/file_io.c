#include "file_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_file(char *filename)
{
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL)
  {
    fprintf(stderr, "Failed to open file. Please check file name or path to file.\n");
		fprintf(stderr, "Provided filename: %s\n", filename);
    exit(1);
  }

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  rewind(fp);

  char *buf = malloc(size + 1);
  if (buf == NULL)
  {
    fprintf(stderr, "failed to allocate buffer. See *read_file(char*)");
    exit(1);
  }

  size_t n = fread(buf, 1, size, fp);
  buf[n] = '\0';

  fclose(fp);

  return buf;
}
