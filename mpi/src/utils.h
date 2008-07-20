#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>

#include <math.h>
#include <mpi.h>

/*
 * Finaliza el entorno MPI y termina el programa
 */
#define MPI_Exit(errnum) MPI_Finalize(), exit(errnum)

/*
 * Niveles de errores.
 */
enum {INFO, WARN, FATAL};
#define err_str(e) (e==INFO ? "INFO" : e==WARN ? "WARN" : e==FATAL ? "FATAL" : "UNKNOWN")

/*
 * Buffer para construir un mensaje de error y
 * luego imprimirlo en una sola llamada a la
 * función fprintf.
 */
#define ERR_BUFF_SIZE 1024
char error_buffer[ERR_BUFF_SIZE + 1];

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
 * Idem que la macro LOG, solo que se utiliza
 * MPI_Exit para terminar el programa.
 */
#define MPI_Log(level, ...)	{ \
                            sprintf(error_buffer, "%s: ", err_str(level)), \
                            sprintf(error_buffer + strlen(error_buffer), __VA_ARGS__), \
                            sprintf(error_buffer + strlen(error_buffer), "\n"), \
							fprintf(stderr, error_buffer, strlen(error_buffer)); \
                            if (level == FATAL || level == WARN) { \
                            	fprintf(stderr, "Archivo: %s\n", __FILE__); \
								fprintf(stderr, "Linea  : %d\n", __LINE__); \
								perror("Error  "); \
                            }; \
                            if (level == FATAL) \
								MPI_Exit(errno); \
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
bool is_positive_integer(char *str);

/*
 * Verifica si un número entero es un cuadrado perfecto.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Un cuadrado perfecto es un número cuya raíz cuadrada
 * es un número entero.
 *
 * Formalmente, x es un cuadrado perfecto si sqrt(x)
 * pertenece al conjunto de los números enteros.
 *
 * Fuente: http://es.wikipedia.org/wiki/Cuadrado_perfecto
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
bool is_perfect_square(int num);

/*
 * Verifica si un número entero es un cubo perfecto.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * A perfect cube (also called a cube number, or 
 * sometimes just a cube) is a number which is the 
 * cube of an integer.
 *
 * Fuente: http://en.wikipedia.org/wiki/Cube_(arithmetic)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
bool is_perfect_cube(int num);

/*
 * Wrapper para la función xmalloc. Es útil
 * ya que no se requiere realizar cast.
 */
#define GET_MEM(type, blocks) (type *) xmalloc(blocks * sizeof(type))

/*
 * Conversión de un número a double. Normalmente
 * útil para realizar divisiones.
 */
#define DOUBLE(x) ((double) x)

/*
 * Conversión de un número a integer. Normalmente
 * útil para realizar truncamientos.
 */
#define INTEGER(x) ((int) x)

/*
 * Imprime el tiempo transcurrido para un
 * programa MPI.
 */
void print_parallel_time(double initTime, double endTime);

/*
 * Función wrapper sobre la función "fopen", para el 
 * chequeo de posibles errores en la apertura de archivos.
 */
FILE *open_file(char *path, char *modo);

#endif /*UTILS_H_*/
