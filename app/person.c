#include "person.h"

FILE* initPersonDB(PersonMeta* meta)
{
  FILE* fp = fopen("people.db", "r+b");

  if (!fp)
  {
    fp = fopen("people.db", "w+b");
    if (!fp)
    {
      perror("Impossibile aprire/creare people.db");
      return EXIT_FAILURE;
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

void freePerson(Person* person)
{
  free(person->name);
}
