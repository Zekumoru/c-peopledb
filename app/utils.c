#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

char* getline()
{
  size_t size = 1;
  size_t capacity = 1;
  char* line = (char*)malloc(capacity);

  // Get each character until newline
  int c;
  while ((c = getchar()) != '\n' && c != '\r' && c != EOF)
  {
    // Reallocate capacity when necessary
    if (size >= capacity)
    {
      capacity *= 2;
      line = (char*)realloc(line, capacity);
    }

    line[size - 1] = c;

    size++;
  }

  line[size - 1] = '\0';

  return line;
}
