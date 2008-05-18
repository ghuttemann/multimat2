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
							fprintf(stderr, "%s\n", err_str(level)), \
                            fprintf(stderr, __VA_ARGS__), \
                            fprintf(stderr, "\n"); \
                            if (level == FATAL || level == WARN) { \
                            	fprintf(stderr, "Archivo: %s\n", __FILE__); \
								fprintf(stderr, "Linea  : %d\n", __LINE__); \
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
 * Verifica si una cadena dada está compuesta
 * únicamente por digitos (0-9), es decir, si
 * la cadena representa un numero entero positivo.
 */
bool is_number(char *str);

/*
 * Wrapper para la función xmalloc. Es útil
 * ya que no se requiere realizar cast.
 */
#define GET_MEM(type, blocks) (type *) xmalloc(blocks * sizeof(type))

#endif /*UTILS_H_*/
