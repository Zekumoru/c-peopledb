#ifndef PERSON_H
#define PERSON_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct PersonMeta
{
  size_t autoIncrementId = 0;
  size_t count = 0;
} PersonMeta;

typedef struct Person
{
  size_t id = 0;
  int age = 0;
  char* name = NULL;
} Person;

FILE* initPersonDB(PersonMeta* meta);

void updatePersonMeta(FILE* fp, PersonMeta* meta);

void loadPersonMeta(FILE* fp, PersonMeta* meta);

Person* readPeople(FILE* fp);

void insertPerson(FILE* fp, Person* person, PersonMeta* meta);

Person* findPerson(FILE* fp, const char* name);

void freePerson(Person* person);

void freePeople(Person* people, size_t size);

#endif // PERSON_H
