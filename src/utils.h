#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

#include <pthread.h>
#include <sqlite3.h>

/*
 * Niveles de errores.
 */
enum {INFO, WARN, FATAL};
#define err_str(e) (e==INFO ? "INFO" : e==WARN ? "WARN" : e==FATAL ? "FATAL" : "UNKNOWN")

/*
 * Despliega un mensaje de error, advertencia,
 * información, etc., y posiblemente termina
 * la ejecución del programa.
 */
#define LOG(level, ...)	{ \
							printf("%s\n", err_str(level)), \
                            printf(__VA_ARGS__), \
                            printf("\n"); \
                            if (level == FATAL || level == WARN) { \
                            	printf("Archivo: %s\n", __FILE__); \
								printf("Linea  : %d\n", __LINE__); \
								perror("Error  "); \
                            }; \
                            if (level == FATAL) \
								exit(errno); \
						}

/*
 * Asigna un bloque de memoria y
 * verifica los posibles fallos.
 */
void *xmalloc(size_t size);

/*
 * Elimina los caracteres de espacio
 * que estén al inicio y final de la
 * cadena.
 */
void str_trim(char *str);

/*
 * Separa la cadena str en dos pezados,
 * separando en la primera ocurrencia
 * de delim y ubicando las partes en part1
 * y part2. Si no se encuentra el delimitador
 * Se retorna str en part1 y en part2 se
 * retorna NULL.
 */ 
void str_split(char *str, char *delim, char *part1, char *part2);

/*
 * Verifica si una cadena dada está compuesta
 * únicamente por digitos.
 */
bool is_number(char *str);

/*
 * Wrapper para la función xmalloc. Es útil
 * ya que no se requiere realizar cast.
 */
#define GET_MEM(type, blocks) (type *) xmalloc(blocks * sizeof(type))

#endif /*UTILS_H_*/
