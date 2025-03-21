/**
 * @file person.h
 * @brief Gestione di una database di persone.
 */

#ifndef PERSON_H
#define PERSON_H

#include "json-parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @struct PersonMeta
 * @brief Metadati per il database delle persone.
 *
 * @var autoIncrementId
 * Identificatore univoco incrementale.
 * @var count
 * Numero di persone nel database.
 */
typedef struct PersonMeta
{
  size_t autoIncrementId;
  size_t count;
} PersonMeta;

/**
 * @struct Person
 * @brief Rappresentazione di una persona.
 *
 * @var id
 * Identificatore unico della persona.
 * @var age
 * Età della persona.
 * @var name
 * Nome della persona.
 */
typedef struct Person
{
  size_t id;
  int age;
  char* name;
} Person;

/**
 * @brief Inizializza il database delle persone.
 *
 * Se il database non esiste, viene creato. In caso contrario, vengono caricati i metadati.
 *
 * @param meta Puntatore ai metadati da utilizzare o aggiornare.
 * @return Puntatore al file del database.
 */
FILE* initPersonDB(PersonMeta* meta);

/**
 * @brief Aggiorna i metadati del database.
 *
 * Scrive i metadati correnti all'inizio del file del database.
 *
 * @param fp Puntatore al file del database.
 * @param meta Puntatore ai metadati da aggiornare.
 */
void updatePersonMeta(FILE* fp, PersonMeta* meta);

/**
 * @brief Carica i metadati dal file del database.
 *
 * Legge i metadati dall'inizio del file e li memorizza nella struttura fornita.
 *
 * @param fp Puntatore al file del database.
 * @param meta Puntatore alla struttura in cui caricare i metadati.
 */
void loadPersonMeta(FILE* fp, PersonMeta* meta);

/**
 * @brief Legge tutte le persone dal database.
 *
 * Carica le persone memorizzate nel database in un array dinamico.
 *
 * @param fp Puntatore al file del database.
 * @return Puntatore all'array delle persone lette.
 */
Person* readPeople(FILE* fp);

/**
 * @brief Inserisce una nuova persona nel database.
 *
 * Assegna un ID univoco alla persona e aggiorna i metadati.
 *
 * @param fp Puntatore al file del database.
 * @param person Puntatore alla persona da inserire.
 * @param meta Puntatore ai metadati.
 */
void insertPerson(FILE* fp, Person* person, PersonMeta* meta);

/**
 * @brief Trova una persona nel database tramite ID.
 *
 * @param fp Puntatore al file del database.
 * @param id ID della persona da trovare.
 * @return Puntatore alla persona trovata, o NULL se non esiste.
 */
Person* findPersonById(FILE* fp, const size_t id);

/**
 * @brief Trova una persona nel database tramite nome.
 *
 * @param fp Puntatore al file del database.
 * @param name Nome della persona da trovare.
 * @return Puntatore alla persona trovata, o NULL se non esiste.
 */
Person* findPerson(FILE* fp, const char* name);

/**
 * @brief Elimina una persona dal database tramite ID.
 *
 * Crea un nuovo file di database senza la persona specificata.
 *
 * @param fpPtr Puntatore al puntatore del file del database.
 * @param meta Puntatore ai metadati.
 * @param id ID della persona da eliminare.
 * @return true se la persona è stata eliminata con successo, false altrimenti.
 */
bool deletePerson(FILE** fpPtr, PersonMeta* meta, const size_t id);

/**
 * @brief Aggiorna una persona nel database.
 *
 * Elimina la persona con l'ID specificato e inserisce i dati aggiornati.
 *
 * @param fpPtr Puntatore al puntatore del file del database.
 * @param meta Puntatore ai metadati.
 * @param id ID della persona da aggiornare.
 * @param updatedPerson Puntatore alla persona aggiornata.
 * @return true se l'aggiornamento è riuscito, false altrimenti.
 */
bool updatePerson(FILE** fpPtr, PersonMeta* meta, const size_t id, Person* updatedPerson);

/**
 * @brief Converte un database di persone in formato JSON e lo scrive su file.
 *
 * @param fp Puntatore al file in cui scrivere il JSON.
 * @param filename Nome del file di output.
 * @return true se la conversione ha avuto successo, false in caso di errore.
 *
 * @note La funzione scrive i dati del database delle persone nel
 *       file specificato in formato JSON.
 */
bool personDbToJson(FILE* fp, const char* filename);

/**
 * @enum PersonJsonError
 * @brief Enumerazione degli errori possibili durante la conversione
 *        dal file JSON alla db.
 */
typedef enum PersonJsonError
{
  NO_PERSON_JSON_ERROR = 0,
  CANNOT_CREATE_PERSON_DB_FILE,
  INVALID_PERSON_JSON_OBJECT,
  EXPECTED_JSON_OBJECT,
  EXPECTED_METADATA_OBJECT,
  EXPECTED_METADATA_AUTO_ID,
  EXPECTED_METADATA_COUNT,
  EXPECTED_PEOPLE_ARRAY,
  EXPECTED_PERSON_OBJECT,
  EXPECTED_PERSON_ID,
  EXPECTED_PERSON_AGE,
  EXPECTED_PERSON_NAME
} PersonJsonError;

/**
 * @brief Carica un database di persone da un file JSON.
 *
 * @param fpPtr Puntatore al puntatore del file del database.
 * @param meta Puntatore ai metadati.
 * @param rootNode Puntatore alla radice dell'albero JSON da
 *                 cui estrarre i dati.
 * @return Un valore della enumerazione PersonJsonError che indica
 *         il risultato dell'operazione.
 */
PersonJsonError loadPersonDbFromJson(FILE** fpPtr, PersonMeta* meta, JsonNode* rootNode);

/**
 * @brief Stampa l'elenco delle persone.
 *
 * Visualizza una tabella con ID, nome ed età delle persone.
 *
 * @param people Puntatore all'array di persone.
 * @param size Numero di persone nell'array.
 */
void printPeople(Person* people, size_t size);

/**
 * @brief Libera la memoria allocata per una persona.
 *
 * @param person Puntatore alla persona da liberare.
 */
void freePerson(Person* person);

/**
 * @brief Libera la memoria allocata per un array di persone.
 *
 * @param people Puntatore all'array di persone.
 * @param size Numero di persone nell'array.
 */
void freePeople(Person* people, size_t size);

#endif // PERSON_H
