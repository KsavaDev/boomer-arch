#include "file_io.h"
#include <stdio.h>
#include <stdlib.h>

char *read_file(char *filename)
{
  FILE *fp = fopen(filename, "r");
  if (fp == NULL)
  {
    fprintf(stderr, "Failed to open file. Please check file name or path to file.");
    exit(1);
  }

  char buf[2048];

  while (fgets(buf, sizeof(buf), fp))
    printf("%s", buf);
  
  fclose(fp);

  return buf;
}
