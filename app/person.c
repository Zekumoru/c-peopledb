#include "person.h"
#include <string.h>

FILE* initPersonDB(PersonMeta* meta)
{
  FILE* fp = fopen("people.db", "r+b");

  if (!fp)
  {
    fp = fopen("people.db", "w+b");
    if (!fp)
    {
      perror("Impossibile aprire/creare people.db");
      return fp;
    }
  }

  // Create meta data if db is newly created otherwise fetch it
  fseek(fp, 0, SEEK_END);
  if (ftell(fp) == 0)
  {
    updatePersonMeta(fp, meta);
  }
  else
  {
    loadPersonMeta(fp, meta);
  }

  fseek(fp, 0, SEEK_END);

  return fp;
}

void updatePersonMeta(FILE* fp, PersonMeta* meta)
{
  size_t oldCursor = ftell(fp);

  fseek(fp, 0, SEEK_SET);
  fwrite(meta, sizeof(PersonMeta), 1, fp);

  fseek(fp, oldCursor, SEEK_SET);
}

void loadPersonMeta(FILE* fp, PersonMeta* meta)
{
  size_t oldCursor = ftell(fp);

  fseek(fp, 0, SEEK_SET);
  fread(meta, sizeof(PersonMeta), 1, fp);

  fseek(fp, oldCursor, SEEK_SET);
}

size_t seekToFirstPerson(FILE* fp)
{
  fseek(fp, sizeof(PersonMeta), SEEK_SET);
  return ftell(fp);
}

char* getPersonName(FILE* fp)
{
  size_t nameLength;
  fread(&nameLength, sizeof(size_t), 1, fp);
  char* name = (char*)malloc(nameLength);
  fread(name, sizeof(char), nameLength, fp);
  return name;
}

void loadPerson(FILE* fp, Person* person)
{
  fread(&person->id, sizeof(size_t), 1, fp);
  fread(&person->age, sizeof(int), 1, fp);
  person->name = getPersonName(fp);
}

Person* readPeople(FILE* fp, PersonMeta* meta)
{
  seekToFirstPerson(fp);
  Person* people = (Person*)malloc(sizeof(Person) * meta->count);
  for (size_t i = 0; i < meta->count; i++)
  {
    loadPerson(fp, &people[i]);
  }

  return people;
}

void insertPerson(FILE* fp, Person* person, PersonMeta* meta)
{
  person->id = meta->autoIncrementId;
  meta->count++;
  meta->autoIncrementId++;
  updatePersonMeta(fp, meta);

  fwrite(&person->id, sizeof(size_t), 1, fp);

  fwrite(&person->age, sizeof(int), 1, fp);

  // Save name
  size_t nameLength = strlen(person->name) + 1; // +1 because of '\0'
  fwrite(&nameLength, sizeof(size_t), 1, fp);
  fwrite(person->name, sizeof(char), nameLength, fp);
}

Person* findPerson(FILE* fp, const char* name)
{
  fseek(fp, 0, SEEK_END);
  const size_t end = ftell(fp);
  seekToFirstPerson(fp);

  while (ftell(fp) < end)
  {
    Person* person = (Person*)malloc(sizeof(Person));
    loadPerson(fp, person);
    if (strcmp(person->name, name) == 0)
    {
      return person;
    }
    freePerson(person);
    free(person);
  }

  return NULL;
}

void freePerson(Person* person)
{
  free(person->name);
}

void freePeople(Person* people, size_t size)
{
  for (size_t i = 0; i < size; i++)
  {
    freePerson(&people[i]);
  }
}
