#include "matrix.h"

/*
 * Rango de cantidad de argumentos.
 */
#define MIN_ARGS_COUNT 6
#define MAX_ARGS_COUNT 10

/*
 * Máxima cantidad de hilos.
 */
#define MAX_THREADS 25

/*
 * Tamaño para las matrices en el 
 * caso de ejemplo.
 */
#define SIZE_EXAMPLE 3

/*
 * Tipo de datos que agrupa
 * los parametros del programa.
 */
typedef struct {
	int matrix_a_fil, matrix_a_col;
	int matrix_b_fil, matrix_b_col;
	int thread_count, distrib_type;
	bool matrix_a_sizes_read;
	bool matrix_b_sizes_read;
	bool thread_count_read;
	bool distrib_type_read;
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
void set_params(param_t *params, int argc, char **argv);

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
