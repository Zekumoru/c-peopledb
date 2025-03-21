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
 * - Salvare il db a un file JSON.
 * - Caricare il db da un file JSON.
 */
#include "app/json-parser.h"
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

typedef enum MenuOption
{
  NO_CHOSEN_OPTION = 0,
  CREATE_PERSON_OPTION,
  FIND_PERSON_OPTION,
  LIST_PEOPLE_OPTION,
  DELETE_PERSON_OPTION,
  UPDATE_PERSON_OPTION,
  SAVE_TO_JSON_OPTION,
  LOAD_JSON_OPTION,
  EXIT_OPTION,
} MenuOption;

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
    case CREATE_PERSON_OPTION:
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
    case FIND_PERSON_OPTION:
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
    case LIST_PEOPLE_OPTION:
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
    case DELETE_PERSON_OPTION:
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
    case UPDATE_PERSON_OPTION:
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
    case SAVE_TO_JSON_OPTION:
    {
      printf("Inserisci il nome per il file JSON da salvare (non aggiungere l'estensione .json): ");
      char* filename = getln();
      filename = (char*)realloc(filename, strlen(filename) + 5);
      strcat(filename, ".json");

      if (personDbToJson(fp, filename))
      {

        printf("\nFile JSON salvato!\n");
      }
      else
      {
        perror("\nErrore: Non riesce salvare il file JSON\n");
      }

      free(filename);
      break;
    }
    case LOAD_JSON_OPTION:
    {
      printf("Inserisci il nome del file JSON da caricare: ");
      char* filename = getln();

      FILE* jsonFile = fopen(filename, "r");

      if (!jsonFile)
      {
        printf("\nErrore: Non riesce aprire il file JSON '%s'\n", filename);
        free(filename);
        break;
      }

      free(filename);

      LexError lexError;
      TokenManager* manager = lex(jsonFile, &lexError);
      if (lexError.type != NO_LEX_ERROR)
      {
        printLexError(&lexError);
        break;
      }

      ParserError parserError;
      parserError.type = NO_PARSER_ERROR;
      JsonNode* root = parse(jsonFile, manager, &parserError);
      if (parserError.type != NO_PARSER_ERROR)
      {
        printParseError(&parserError);
        break;
      }

      PersonJsonError errorCode = loadPersonDbFromJson(&fp, &meta, root);
      if (errorCode != NO_PERSON_JSON_ERROR)
      {
        printf("\nErrore: Non riesce caricare il file JSON, verificare che il sintasso del file sia giusto.\n");
        remove("people_temp.db");
      }
      else
      {
        printf("\nCaricato file JSON nel DB con successo!\n");
      }

      freeJsonTree(root);
      fclose(jsonFile);
      break;
    }
    case EXIT_OPTION:
      printf("Arrivederci!\n");
      break;
    default:
      printf("Opzione non valida. Riprova.\n");
      break;
    }

    if (choice != EXIT_OPTION)
    {
      printf("\n");
      pause();
    }
  } while (choice != EXIT_OPTION);

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
  printf("6. Salvare tutte le persone in JSON\n");
  printf("7. Caricare persone da un file JSON\n");
  printf("   (ATTENTO: Questa operazione sostituisce l'attuale db)\n");
  printf("8. Esci\n");
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
