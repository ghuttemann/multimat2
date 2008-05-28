#include "matrix.h"

/*
 * Rango de cantidad de argumentos.
 */
#define MIN_ARGS_COUNT 6
#define MAX_ARGS_COUNT 11

/*
 * Máxima cantidad de hilos.
 */
#define MAX_THREADS 100

/*
 * Tamaño para las matrices en el 
 * caso de ejemplo.
 */
#define SIZE_EXAMPLE 3

/*
 * Nombre del archivo de salida, en
 * el que se imprimiran las matrices.
 */
#define OUTPUT_FILE "matrix-mult_matrices.csv"

/*
 * Nombre del archivo de salida, en
 * el que se imprimiran los tiempos.
 */
#define TIMES_FILE "matrix-mult_tiempos.csv"

/*
 * Nombre del archivo de salida, en
 * el que se imprimiran las particiones.
 */
#define PARTIT_FILE "matrix-mult_particiones.csv"

/*
 * Tipo de datos que agrupa
 * los parametros del programa.
 */
typedef struct {
	int matrix_a_fil, matrix_a_col;
	int matrix_b_fil, matrix_b_col;
	int thread_count, distrib_type;
} param_t;

/*
 * Imprime una ayuda de cómo se debe
 * utilizar el programa y termina.
 */
void como_usar(void);

/*
 * Recorre la lista de argumentos y
 * los almacena en la estructura de
 * parametros.
 */
void set_params(param_t *params, int argc, char **argv, bool *thread_count_read,
				bool *print_output);

/*
 * Ajusta la cantidad de hilos a una
 * cantidad apropiada, considerando
 * un máximo global o un máximo que
 * depende del tamaño de la matriz.
 */
void adjust_thread_count(param_t *params);

/*
 * Función de multiplicación para los hilos.
 */
void *matrix_mult_thread(void *args);

/*
 * Realiza la distribución de las matrices con
 * particionamiento 1-D (una dimensión).
 * El esquema de particionamiento de datos utilizado
 * es el de "Datos de Salida".
 */
void distrib_1d(matrix_t *mat_a, matrix_t *mat_b, matrix_t *mat_c, 
		int thread_count, matrix_mult_args *arguments);

/*
 * Realiza la distribución de las matrices con
 * particionamiento 2-D (dos dimensiones).
 * El esquema de particionamiento de datos utilizado
 * es el de "Datos de Salida".
 */
void distrib_2d(matrix_t *mat_a, matrix_t *mat_b, matrix_t *mat_c, 
		int thread_count, matrix_mult_args *arguments);

/*
 * Imprime las matrices de entrada y salida en un
 * archivo de texto.
 */
void print_matrices(matrix_t *mat_a, matrix_t *mat_b, matrix_t *mat_c);

/*
 * Imprime los tiempos calculados.
 */
void print_times(time_rec_t tiempo_total_multip, time_rec_t tiempo_total_partit,
			time_rec_t tiempo_total_thr_creat, time_rec_t tiempo_total_thr_exec,
			int thread_count);

/*
 * Imprime las particiones de cada hilo.
 */
void print_partitions(matrix_mult_args *arguments, int thread_count);
