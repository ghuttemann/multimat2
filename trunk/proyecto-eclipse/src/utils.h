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

#include <sys/time.h>
#include <unistd.h>

#include <math.h>
#include <pthread.h>

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
							fprintf(stderr, "%s: ", err_str(level)), \
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
 * únicamente por digitos decimales (0-9), 
 * es decir, si la cadena representa un numero 
 * entero positivo.
 */
bool is_number(char *str);

/*
 * Verifica si un número entero es un cuadrado perfecto.
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Un cuadrado perfecto es un número cuya raíz cuadrada 
 * es un número entero.
 * Formalmente, x es un cuadrado perfecto si sqrt(x) 
 * pertenece al conjunto de los números enteros.
 * 
 * Fuente: http://es.wikipedia.org/wiki/Cuadrado_perfecto
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
bool es_cuadrado_perfecto(int numero);

/*
 * Wrapper para la función xmalloc. Es útil
 * ya que no se requiere realizar cast.
 */
#define GET_MEM(type, blocks) (type *) xmalloc(blocks * sizeof(type))

/*
 * Función que retorna el tiempo transcurrido
 * desde Epoch (ver time(2)).
 * La unidad de medida está en "milisegundos".
 */
long long get_time_millis(void);

/* 
 * Tipo de dato para guardar
 * registros de tiempo.
 */
typedef struct {
	long long begin;
	long long end;
} time_rec_t;

/*
 * Macros para facilitar el control de
 * tiempo.
 */
#define TIME_BEGIN(x)  x.begin = get_time_millis()
#define TIME_END(x)    x.end   = get_time_millis()
#define TIME_DIFF(x)   (x.end - x.begin)

/*
 * Conversión de un número a double. Normalmente
 * útil para realizar divisiones.
 */
#define DOUBLE(x) ((double) x)

#endif /*UTILS_H_*/
