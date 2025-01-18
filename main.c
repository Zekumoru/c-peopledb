/**
 * Applicazione per la gestione di un database di persone che è salvato in
 * un file binario.
 *
 * Questo programma consente di interagire con un database di persone tramite
 * un menu interattivo. Le funzionalità includono:
 * - Creare una nuova persona.
 * - Trovare una persona per ID.
 * - Visualizzare tutte le persone.
 * - Eliminare una persona.
 * - Aggiornare una persona esistente.
 */
#include "app/person.h"
#include "app/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Stampa il menu principale dell'applicazione.
 *
 * La funzione mostra all'utente un elenco di opzioni per interagire
 * con il database delle persone. Ogni opzione corrisponde a una
 * funzionalità specifica che l'utente può selezionare.
 */
void printMenu();

/**
 * @brief Richiede all'utente di inserire un'età valida e verifica
 *        che sia all'interno di un intervallo specificato.
 *
 * La funzione continua a chiedere l'input finché non viene
 * fornita un'età valida.
 *
 * @return int Un'età valida.
 */
int getValidAge();

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
      char* name = getln();
      printf("Inserisci l'et\u00e0 della persona: ");
      int age = getValidAge();

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
        char* newName = getln();
        printf("Inserisci la nuova et\u00e0 della persona (vecchio: %d): ", person->age);
        int newAge = getValidAge();

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

int getValidAge()
{
  const int min = 1;
  const int max = 999;

  int age;
  do
  {
    age = getint();

    if (age < min || age > max)
    {
      printf("L'et\u00e0 deve essere tra %d e %d. Riprova.\n", min, max);
    }
    else
    {
      break;
    }
  } while (1);

  return age;
}
