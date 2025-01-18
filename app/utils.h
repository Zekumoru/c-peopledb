#ifndef UTILS_H
#define UTILS_H

/**
 * @brief Legge una riga di input dallo standard input (stdin) fino a
 *        incontrare un carattere di newline o carriage return.
 *
 * Questa funzione alloca dinamicamente la memoria per memorizzare la
 * riga di input e aumenta la dimensione del buffer se necessario.
 * La stringa restituita è terminata da un carattere null (`\0`).
 *
 * @return char* Puntatore alla stringa allocata dinamicamente
 *               contenente la riga di input.
 *
 * @note La funzione non include il carattere di newline o carriage return
 *       nella stringa restituita.
 *
 * @warning Il chiamante è responsabile di liberare la memoria
 *          utilizzando `free()` quando non è più necessaria.
 */
char* getline();

/**
 * @brief Legge un intero dallo standard input (stdin).
 *
 * La funzione richiede all'utente di inserire un valore numerico intero.
 *
 * @return int Valore intero inserito dall'utente.
 *
 * @note Se l'input è invalido, restituisce 0.
 */
int getint();

/**
 * @brief Legge un numero in virgola mobile dallo standard input (stdin).
 *
 * La funzione richiede all'utente di inserire un valore numerico in
 * virgola mobile.
 *
 * @return double Valore in virgola mobile inserito dall'utente.
 *
 * @note Se l'input è invalido, restituisce 0.
 */
double getdbl();

#endif // UTILS_H
