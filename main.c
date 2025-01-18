#include "app/person.h"
#include "app/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printMenu()
{
  clearScreen();
  printf("--- Menu | PeopleDB ---\n");
  printf("1. Crea una nuova persona\n");
  printf("2. Trova una persona per ID\n");
  printf("3. Visualizza tutte le persone\n");
  printf("4. Elimina una persona\n");
  printf("5. Aggiorna una persona esistente\n");
  printf("6. Esci\n");
  printf("Scegli un'opzione: ");
}

int main()
{
  FILE* fp = NULL;
  PersonMeta meta = {0};
  fp = initPersonDB(&meta);

  if (!fp)
  {
    fprintf(stderr, "Errore nell'apertura del database.\n");
    return 1;
  }

  int choice;
  do
  {
    printMenu();
    choice = getint();

    clearScreen();

    switch (choice)
    {
    case 1:
    {
      printf("Crea una nuova persona\n\n");
      printf("Inserisci il nome della persona: ");
      char* name = getline();
      printf("Inserisci l'et\u00e0 della persona: ");
      int age = getint();

      Person person = {0, age, name};
      insertPerson(fp, &person, &meta);
      free(name);
      printf("\nPersona aggiunta con successo!\n");

      break;
    }
    case 2:
    {
      printf("Trova una persona per ID\n\n");
      printf("Inserisci l'ID della persona: ");
      size_t id = (size_t)getint();
      printf("\n");

      Person* person = findPersonById(fp, id);
      if (person)
      {
        printf("Persona trovata:\nID: %zu\nNome: %s\nEt\u00e0: %d\n", person->id, person->name, person->age);
        freePerson(person);
        free(person);
      }
      else
      {
        printf("Persona non trovata.\n");
      }

      break;
    }
    case 3:
    {
      printf("Visualizza tutte le persone\n\n");
      Person* people = readPeople(fp);
      if (meta.count > 0)
      {
        printPeople(people, meta.count);
        freePeople(people, meta.count);
        free(people);
      }
      else
      {
        printf("Nessuna persona trovata.\n");
      }

      break;
    }
    case 4:
    {
      printf("Elimina una persona\n\n");
      printf("Inserisci l'ID della persona da eliminare: ");
      size_t id = (size_t)getint();

      printf("\n");

      if (deletePerson(&fp, &meta, id))
      {
        printf("Persona eliminata con successo!\n");
      }
      else
      {
        printf("Persona non trovata.\n");
      }

      break;
    }
    case 5:
    {
      printf("Aggiorna una persona esistente\n\n");
      printf("Inserisci l'ID della persona da aggiornare: ");
      size_t id = (size_t)getint();

      printf("\n");

      Person* person = findPersonById(fp, id);
      if (person)
      {
        printf("Inserisci il nuovo nome della persona (vecchio: %s): ", person->name);
        char* newName = getline();
        printf("Inserisci la nuova et\u00e0 della persona (vecchio: %d): ", person->age);
        int newAge = getint();

        Person updatedPerson = {id, newAge, newName};
        updatePerson(&fp, &meta, id, &updatedPerson);
        free(newName);

        printf("\nPersona aggiornata con successo!\n");
      }
      else
      {
        printf("\nPersona non trovata.\n");
      }

      break;
    }
    case 6:
      printf("Arrivederci!\n");
      break;
    default:
      printf("Opzione non valida. Riprova.\n");
      break;
    }

    if (choice != 6)
    {
      printf("\n");
      pause();
    }
  } while (choice != 6);

  fclose(fp);
  return 0;
}
