#include "app/utils.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
  char* input;

  printf("Enter string: ");
  input = getline();

  printf("You entered: %s\n", input);

  free(input);

  return 0;
}
