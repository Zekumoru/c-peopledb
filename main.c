#include "app/person.h"
#include "app/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
  PersonMeta personMeta;
  FILE* fp = initPersonDB(&personMeta);

  Person person;

  printf("Enter name: ");
  person.name = getline();

  printf("Enter age: ");
  person.age = getint();

  insertPerson(fp, &person, &personMeta);

  freePerson(&person);

  fclose(fp);

  return 0;
}
