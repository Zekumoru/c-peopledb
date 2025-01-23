#include "person.h"
#include "utils.h"
#include <stdlib.h>
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

const size_t getEndAndSeekToFirstPerson(FILE* fp)
{
  fseek(fp, 0, SEEK_END);
  const size_t end = ftell(fp);
  const size_t start = seekToFirstPerson(fp);
  return end;
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

Person* readPeople(FILE* fp)
{
  const size_t end = getEndAndSeekToFirstPerson(fp);
  Person* people = (Person*)malloc(end - ftell(fp));
  for (size_t i = 0; ftell(fp) < end; i++)
  {
    loadPerson(fp, &people[i]);
  }

  return people;
}

void insertPerson(FILE* fp, Person* person, PersonMeta* meta)
{
  if (meta != NULL)
  {
    person->id = meta->autoIncrementId;
    meta->count++;
    meta->autoIncrementId++;
    updatePersonMeta(fp, meta);
  }

  fseek(fp, 0, SEEK_END);

  fwrite(&person->id, sizeof(size_t), 1, fp);

  fwrite(&person->age, sizeof(int), 1, fp);

  // Save name
  size_t nameLength = strlen(person->name) + 1; // +1 because of '\0'
  fwrite(&nameLength, sizeof(size_t), 1, fp);
  fwrite(person->name, sizeof(char), nameLength, fp);
}

Person* findPersonById(FILE* fp, const size_t id)
{
  const size_t end = getEndAndSeekToFirstPerson(fp);
  while (ftell(fp) < end)
  {
    Person* person = (Person*)malloc(sizeof(Person));
    loadPerson(fp, person);
    if (person->id == id)
    {
      return person;
    }
    freePerson(person);
    free(person);
  }

  return NULL;
}

Person* findPerson(FILE* fp, const char* name)
{
  const size_t end = getEndAndSeekToFirstPerson(fp);
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

bool deletePerson(FILE** fpPtr, PersonMeta* meta, const size_t id)
{
  FILE* fp = *fpPtr;

  // Check first if it exists before creating a new file to delete
  Person* person = findPersonById(fp, id);
  if (person == NULL)
  {
    return false;
  }

  freePerson(person);
  free(person);

  FILE* newFp = fopen("people_temp.db", "w+b");
  if (!newFp)
  {
    perror("Impossibile creare un nuovo file per rimuovere un elemento");
    return false;
  }

  meta->count--;
  updatePersonMeta(newFp, meta);

  const size_t end = getEndAndSeekToFirstPerson(fp);
  while (ftell(fp) < end)
  {
    Person person;
    loadPerson(fp, &person);
    if (person.id != id)
    {
      insertPerson(newFp, &person, NULL);
    }
    freePerson(&person);
  }

  fclose(fp);
  *fpPtr = newFp;
  remove("people.db");
  rename("people_temp.db", "people.db");

  return true;
}

bool updatePerson(FILE** fpPtr, PersonMeta* meta, const size_t id, Person* updatedPerson)
{
  bool deleted = deletePerson(fpPtr, meta, id);
  if (!deleted)
  {
    return false;
  }

  insertPerson(*fpPtr, updatedPerson, NULL);
  meta->count++;
  return true;
}

bool personDbToJson(FILE* fp, const char* filename)
{
  PersonMeta meta;
  loadPersonMeta(fp, &meta);

  FILE* jsonFile = fopen(filename, "w");
  if (!jsonFile)
    return false;

  fputc('{', jsonFile); // start root object

  fputs("\"metadata\":{", jsonFile);
  fputs("\"autoIncrementId\":", jsonFile);

  char* autoIdStr = size_tToString(meta.autoIncrementId);
  fputs(autoIdStr, jsonFile);
  free(autoIdStr);

  fputc(',', jsonFile);

  fputs("\"count\":", jsonFile);
  char* countStr = size_tToString(meta.count);
  fputs(countStr, jsonFile);
  free(countStr);

  fputc('}', jsonFile); // end metadata object
  fputc(',', jsonFile);

  fputs("\"people\":[", jsonFile);

  const size_t end = getEndAndSeekToFirstPerson(fp);
  while (ftell(fp) < end)
  {
    Person person;
    loadPerson(fp, &person);

    fputc('{', jsonFile); // start person object

    fputs("\"id\":", jsonFile);
    char* idStr = size_tToString(person.id);
    fputs(idStr, jsonFile);
    free(idStr);
    fputc(',', jsonFile);

    fputs("\"age\":", jsonFile);
    char* ageStr = intToString(person.age);
    fputs(ageStr, jsonFile);
    free(ageStr);
    fputc(',', jsonFile);

    fputs("\"name\":", jsonFile);
    fputc('"', jsonFile);
    fputs(person.name, jsonFile);
    fputc('"', jsonFile);

    fputc('}', jsonFile); // end person object

    freePerson(&person);
  }

  fputc(']', jsonFile); // end people array

  fputc('}', jsonFile); // end root object

  fclose(jsonFile);
  return true;
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
