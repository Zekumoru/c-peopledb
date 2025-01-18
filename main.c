#include "app/person.h"
#include "app/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
  PersonMeta personMeta;
  FILE* fp = initPersonDB(&personMeta);
  if (!fp)
  {
    return EXIT_FAILURE;
  }

  fclose(fp);

  return 0;
}
