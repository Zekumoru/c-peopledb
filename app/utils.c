#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* getln()
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

int getint()
{
  char* input = getln();
  int num = atoi(input);
  free(input);
  return num;
}

double getdbl()
{
  char* input = getln();
  double num = strtod(input, NULL);
  free(input);
  return num;
}

void clearScreen()
{
  printf("\033[2J\033[1;1H");
}

void pause()
{
  printf("Premere 'invio' per continuare...");
  char* temp = getln();
  free(temp);
}

char* size_tToString(const size_t n)
{
  // Get how many digits using log10 and add 1 for '\0'
  // If n is 0 then length is 2 because there's no log10 of 0
  const size_t length = n == 0 ? 2 : ceil(log10(n)) + (n == 0 ? 2 : 1);
  char* str = (char*)malloc(length);
  sprintf(str, "%ld", n);
  return str;
}

char* intToString(const int n)
{
  // Get how many digits using log10 and add 1 for '\0'
  // Add another 1 if negative for '-'
  // If n is 0 then length is 2 because there's no log10 of 0
  const size_t length = n == 0 ? 2 : ceil(log10(abs(n))) + (n < 0 ? 2 : 1);
  char* str = (char*)malloc(length);
  sprintf(str, "%d", n);
  return str;
}
