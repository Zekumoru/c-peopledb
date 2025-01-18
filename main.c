#include "app/person.h"
#include "app/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void populatePerson(FILE* fp, PersonMeta* meta, const char* name, int age)
{
  Person person;

  person.name = strdup(name);
  person.age = age;

  insertPerson(fp, &person, meta);

  freePerson(&person);
}

void populatePeople(FILE* fp, PersonMeta* meta)
{
  populatePerson(fp, meta, "Foo", 20);
  populatePerson(fp, meta, "Bar", 40);
  populatePerson(fp, meta, "Baz", 30);
}

void printPeople(Person* people, size_t size)
{
  printf("%-5s | %-30s | %-10s\n", "ID", "Name", "Age");

  for (size_t i = 0; i < 5 + 30 + 10 + 3 * 2; i++)
    printf("-");
  printf("\n");

  for (size_t i = 0; i < size; i++)
  {
    Person* person = &people[i];
    printf("%-5ld | %-30s | %-10d\n", person->id, person->name, person->age);
  }
}

int main()
{
  PersonMeta personMeta;
  FILE* fp = initPersonDB(&personMeta);
  if (!fp)
    return EXIT_FAILURE;

  // populatePeople(fp, &personMeta);
  // printf("Database has been populated!\n");

  Person* people = readPeople(fp);
  printPeople(people, personMeta.count);

  freePeople(people, personMeta.count);
  free(people);

  fclose(fp);

  return 0;
}
