#include "matrix.h"

/*
 * Rango de cantidad de argumentos.
 */
#define MIN_ARGS_COUNT 6
#define MAX_ARGS_COUNT 10

/*
 * Máxima cantidad de hilos.
 */
#define MAX_THREADS 20

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
void como_usar() {
	fprintf(stderr, "Argumento invalido\n");
	exit(0);
}

/*
 * Recorre la lista de argumentos y
 * los almacena en la estructura de
 * parametros.
 */
void set_params(param_t *params, int argc, char **argv) {
	bool condicion = false;
	
	if (argc == 2 && strcmp(argv[1], "-e") == 0) {
		// Ejemplo secuencial
		params->matrix_a_fil = SIZE_EXAMPLE;
		params->matrix_a_col = SIZE_EXAMPLE;
		params->matrix_b_fil = SIZE_EXAMPLE;
		params->matrix_b_col = SIZE_EXAMPLE;
		params->thread_count = 0;
		params->distrib_type = 1;
		
		params->matrix_a_sizes_read = true;
		params->matrix_b_sizes_read = true;
		params->thread_count_read  = false;
		params->distrib_type_read  = false;
		
		condicion = true;
	}
	else if (argc == MIN_ARGS_COUNT + 1 || argc == MAX_ARGS_COUNT + 1) {
		/*
		 * Procesamos los argumentos pasados
		 * por línea de comandos.
		 */
		int i;
		for (i=1; i < argc; i++) {
			if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "-b") == 0) {
				/*
				 * Verificar que hayan al menos
				 * dos argumentos más y que estos
				 * sean números enteros.
				 */
				condicion = i + 2 < argc && 
						 	is_number(argv[i + 1]) && 
							is_number(argv[i + 2]);

				if (condicion) {
					if (strcmp(argv[i], "-a") == 0) {
						params->matrix_a_fil = atoi(argv[i + 1]);
						params->matrix_a_col = atoi(argv[i + 2]);
						params->matrix_a_sizes_read = true;
					}
					else {
						params->matrix_b_fil = atoi(argv[i + 1]);
						params->matrix_b_col = atoi(argv[i + 2]);
						params->matrix_b_sizes_read = true;
					}
					
					// Avanzamos el indice
					i += 2;
				}
			}
			else if (strcmp(argv[i], "-h") == 0) {
				/*
				 * Verificar que haya al menos
				 * un argumento más y que este
				 * sea un numero entero. Además,
				 * los tamaños de las matrice ya
				 * se tuvieron que haber leído.
				 */
				condicion = (i + 1 < argc) &&
							is_number(argv[i + 1]) &&
							params->matrix_a_sizes_read &&
							params->matrix_b_sizes_read;

				if (condicion) {
					params->thread_count = atoi(argv[i + 1]);
					params->thread_count_read = true;
					
					// Avanzamos el indice
					i += 1;
				}
			}
			else if (strcmp(argv[i], "-t") == 0) {
				/*
				 * Verificar que haya al menos
				 * un argumento más y que este
				 * sea un numero entero. Además,
				 * la cantidad de hilos ya se tuvo
				 * que haber leído.
				 */
				condicion = (i + 1 < argc) &&
							is_number(argv[i + 1]) &&
							params->thread_count_read;

				if (condicion) {
					params->distrib_type = atoi(argv[i + 1]);
					params->distrib_type_read = true;
					
					// Avanzamos el indice
					i += 1;
					
					// Tipo de distribución 1d o 2d
					if (params->distrib_type != 1 && params->distrib_type != 2)
						condicion = false;
				}
			}
			
			if (!condicion)
				break;
		}
	}
	
	/*
	 * Si la cantidad de argumentos no es
	 * correcta. Se especifica la forma
	 * de utilizar el programa.
	 */	
	if (!condicion)
		como_usar();
}

/*
 * Ajusta la cantidad de hilos a una
 * cantidad apropiada, considerando
 * un máximo global o un máximo que
 * depende del tamaño de la matriz.
 */
void adjust_thread_count(param_t *params) {
	if (params->thread_count > MAX_THREADS)
		params->thread_count = MAX_THREADS;
	
	/* 
	 * TODO: verificar con matrix_sizes
	 */
}

/*
 * Función de multiplicación para los hilos.
 */
void *matrix_mult_thread(void *args) {
	matrix_mult_args *aux = (matrix_mult_args *) args;
	
	matrix_mult(aux->matrix_a, aux->matrix_b, aux->matrix_c, 
				aux->row_begin, aux->row_end, aux->col_begin, aux->col_end);
	
	pthread_exit((void *) 0);
}

/*
 * Realiza la distribución de las matrices con
 * particionamiento 1-D (una dimensión).
 */
void distrib_1d(matrix_t *mat_a, matrix_t *mat_b, matrix_t *mat_c, 
		int thread_count, matrix_mult_args *arguments) {
	
	int i;
	int rows_count = matrix_rows(mat_c) / thread_count;
	int remainder_rows  = matrix_rows(mat_c) % thread_count;
	
	for (i=0; i < thread_count; i++) {
		// A cada uno se les asigna las matrices
		arguments[i].matrix_a  = mat_a;
		arguments[i].matrix_b  = mat_b;
		arguments[i].matrix_c  = mat_c;
		
		// A cada uno se asigna rows_count filas
		arguments[i].row_begin = i * rows_count;
		arguments[i].row_end   = arguments[i].row_begin + rows_count - 1;
		
		// A cada uno se asignan todas las columnas
		arguments[i].col_begin = 0;
		arguments[i].col_end   = matrix_cols(mat_c) - 1;
	}
	
	/*
	 * Si sobraron filas sin asignar, éstas
	 * son asignadas al último hilo.
	 */
	if (remainder_rows > 0)
		arguments[thread_count - 1].row_end += remainder_rows;
}

/*
 * Realiza la distribución de las matrices con
 * particionamiento 2-D (dos dimensiones).
 */
void distrib_2d(matrix_t *mat_a, matrix_t *mat_b, matrix_t *mat_c, 
		int thread_count, matrix_mult_args *arguments) {
	
//	int i;
//	int rows_count = matrix_rows(mat_c) / thread_count;
//	int cols_count = matrix_cols(mat_c) / thread_count;
//	int remainder_rows = matrix_rows(mat_c) % thread_count;
//	int remainder_cols = matrix_cols(mat_c) % thread_count;
//
//	for (i=0; i < thread_count; i++) {
//		// A cada uno se les asigna las matrices
//		arguments[i].matrix_a  = mat_a;
//		arguments[i].matrix_b  = mat_b;
//		arguments[i].matrix_c  = mat_c;
//		
//		// A cada uno se asigna rows_count filas
//		arguments[i].row_begin = i * rows_count;
//		arguments[i].row_end   = arguments[i].row_begin + rows_count - 1;
//		
//		// A cada uno se asignan todas las columnas
//		arguments[i].col_begin = i * cols_count;
//		arguments[i].col_end   = arguments[i].col_begin + cols_count - 1;
//	}
//	
//	/*
//	 * Si sobraron filas sin asignar, éstas
//	 * son asignadas a los hilos que recibieron
//	 * las últimas filas.
//	 */
//	if (remainder_rows > 0) {
//		
//	}
//	
//	/*
//	 * Si sobraron columnas sin asignar, éstas
//	 * son asignadas a los hilos que recibieron
//	 * las últimas columnas.
//	 */
//	if (remainder_cols > 0) {
//		
//	}
}

/*
 * Función principal del programa.
 */
int main(int argc, char **argv) {
	matrix_t *mat_a, *mat_b, *mat_c;
	int i;
	param_t params = {0};
	
	/*
	 * Verificamos si se pasó como argumento
	 * algún archivo de configuración, y 
	 * cargamos los parámentros de configuración.
	 */
	set_params(&params, argc, argv);

	/*
	 * Verificamos que la cantidad de columnas
	 * de A sea igual a la cantidad de filas de B
	 */
	if (params.matrix_a_col != params.matrix_b_fil)
		LOG(FATAL, "%s %s", "La cantidad de filas de la matriz A debe ser",
				"igual a la cantidad de filas de la matriz B.");
	
	/*
	 * Creamos las matrices A, B y C.
	 */
	matrix_create(&mat_a, params.matrix_a_fil, params.matrix_a_col);
	matrix_create(&mat_b, params.matrix_b_fil, params.matrix_b_col);
	matrix_create(&mat_c, matrix_rows(mat_a), matrix_cols(mat_b));
	
	/*
	 * Cargamos las matrices con valores
	 * aleatorios.
	 */
	matrix_fill(mat_a);
	matrix_fill(mat_b);
	
	
	if (params.thread_count_read) {
		/*
		 * Multiplicación concurrentemente. La cantidad
		 * de hilos se debe ajustar apropiadamente.
		 */
		adjust_thread_count(&params);
		
		/*
		 * Creación de hilos
		 */
		pthread_t *threads = GET_MEM(pthread_t, params.thread_count);
		matrix_mult_args *arguments = GET_MEM(matrix_mult_args, params.thread_count);
		
		/*
		 * Realizar distribución de matrices
		 */
		if (params.distrib_type == 1)
			distrib_1d(mat_a, mat_b, mat_c, params.thread_count, arguments);
		else
			distrib_2d(mat_a, mat_b, mat_c, params.thread_count, arguments);
		
		/*
		 * Lanzar los hilos.
		 */
		for (i=0; i < params.thread_count; i++)
			pthread_create(&threads[i], NULL, matrix_mult_thread, &arguments[i]);
		
		/*
		 * Esperar a los hilos.
		 */		
		for (i=0; i < params.thread_count; i++)
			pthread_join(threads[i], NULL);
		
		/*
		 * Realizar limpieza
		 */
		free(threads);
		free(arguments);
	}
	else {
		/*
		 * Multiplicación secuencial.
		 */
		matrix_mult(mat_a, mat_b, mat_c, 
					0, matrix_rows(mat_c) - 1, 
					0, matrix_cols(mat_c) - 1);
	}
	
	/*
	 * Imprimir las matrices
	 */
	printf("Matriz A\n");
	matrix_print(mat_a);
	printf("\n");
	
	printf("Matriz B\n");
	matrix_print(mat_b);
	printf("\n");
	
	printf("Matriz C\n");
	matrix_print(mat_c);
	printf("\n");
	
	/*
	 * Destruimos las matrices.
	 */
	matrix_destroy(mat_a);
	matrix_destroy(mat_b);
	matrix_destroy(mat_c);
	
	return EXIT_SUCCESS;
}
