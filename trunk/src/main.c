#include "matrix.h"

#define MIN_ARGS_COUNT 2

void como_usar() {
	LOG(INFO, "Argumento inválido");
}

void *matrix_mult_thread(void *args) {
	matrix_mult_args *aux = (matrix_mult_args *) args;
	
	matrix_mult(aux->matrix_a, aux->matrix_b, aux->matrix_c, 
				aux->row_begin, aux->row_end, aux->col_begin, aux->col_end);
	
	pthread_exit((void *) 0);
}

void distrib_1d(matrix_t *mat_a, matrix_t *mat_b, matrix_t *mat_c, 
		int thread_count, matrix_mult_args *arguments) {
	
	int i;
	
	for (i=0; i < thread_count; i++) {
		arguments[i].matrix_a  = mat_a;
		arguments[i].matrix_b  = mat_b;
		arguments[i].matrix_c  = mat_c;
		arguments[i].row_begin = i * thread_count;
		arguments[i].row_end   = arguments[i].row_begin + thread_count - 1;
		arguments[i].col_begin = 0;
		arguments[i].col_end   = matrix_cols(mat_c) - 1;
	}
}

void distrib_2d(matrix_t *mat_a, matrix_t *mat_b, matrix_t *mat_c, 
		int thread_count, matrix_mult_args *arguments) {
	
//	int i;
//
//	for (i=0; i < thread_count; i++) {
//		arguments[i].matrix_a  = mat_a;
//		arguments[i].matrix_b  = mat_b;
//		arguments[i].matrix_c  = mat_c;
//		arguments[i].row_begin = i * thread_count;
//		arguments[i].row_end   = arguments[i].row_begin + thread_count - 1;
//		arguments[i].col_begin = 0;
//		arguments[i].col_end   = matrix_cols(mat_c) - 1;
//	}
}

int main(int argc, char **argv) {
	matrix_t *mat_a, *mat_b, *mat_c;
	int i, matrix_size, thread_count, distrib_type;
	bool matrix_size_read = false;
	
	/*
	 * Verificamos si se pasó como argumento
	 * algún archivo de configuración, y 
	 * cargamos los parámentros de configuración.
	 */
	if (argc == 1) {
		// Argumentos por defecto
		matrix_size  = 4;
		thread_count = 0;
		distrib_type = 1;
	}
	else if (argc >= MIN_ARGS_COUNT + 1) {
		/*
		 * Procesamos los argumentos pasados
		 * por línea de comandos.
		 */
		for (i=1; i < argc; i++) {
			if (strcmp(argv[i], "-n")) {
				/*
				 * Verificar que haya al menos
				 * un argumento más y que éste
				 * sea un número entero.
				 */
				if (i + 1 < argc && is_number(argv[i + 1])) {
					matrix_size = atoi(argv[i + 1]);
					matrix_size_read = true;
				}
				else {
					como_usar();
				}
			}
			else if (strcmp(argv[i], "-c")) {
				/*
				 * Verificar que haya al menos
				 * dos argumentos más y que estos
				 * sean numeros enteros. Además,
				 * el tamaño de la matriz ya tuvo
				 * que haberse leído.
				 */
				bool condicion = (i + 2 < argc) && 
								 is_number(argv[i + 1]) && 
								 is_number(argv[i + 2]) &&
								 matrix_size_read;
				
				if (condicion) {
					thread_count = atoi(argv[i + 1]);
					distrib_type = atoi(argv[i + 2]);
				}
				else {
					como_usar();
				}
			}
			else {
				como_usar();
			}
		}
	}
	else {
		/*
		 * La cantidad de argumentos no es
		 * correcta. Se especifica la forma
		 * de utilizar el programa.
		 */
		como_usar();
	}

	
	/*
	 * Creamos las matrices A, B y C.
	 */
	matrix_create(&mat_a, matrix_size, matrix_size);
	matrix_create(&mat_b, matrix_size, matrix_size);
	matrix_create(&mat_c, matrix_rows(mat_a), matrix_cols(mat_b));
	
	/*
	 * Cargamos las matrices con valores
	 * aleatorios.
	 */
	matrix_fill(mat_a);
	matrix_fill(mat_b);
	
	
	if (thread_count > 0) {
		/*
		 * Multiplicación concurrentemente.
		 */

		
		/*
		 * Creación de hilos
		 */
		pthread_t *threads = GET_MEM(pthread_t, thread_count);
		matrix_mult_args *arguments = GET_MEM(matrix_mult_args, thread_count);
		
		/*
		 * Realizar distribución de matrices
		 */
		if (distrib_type == 1)
			distrib_1d(mat_a, mat_b, mat_c, thread_count, arguments);
		else
			distrib_2d(mat_a, mat_b, mat_c, thread_count, arguments);
		
		/*
		 * Lanzar los hilos.
		 */
		for (i=0; i < thread_count; i++)
			pthread_create(&threads[i], NULL, matrix_mult_thread, &arguments[i]);
		
		/*
		 * Esperar a los hilos.
		 */		
		for (i=0; i < thread_count; i++)
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
	matrix_print(mat_a); printf("\n");
	matrix_print(mat_b); printf("\n");
	matrix_print(mat_c); printf("\n");
	
	/*
	 * Destruimos las matrices.
	 */
	matrix_destroy(mat_a);
	matrix_destroy(mat_b);
	matrix_destroy(mat_c);
	
	return EXIT_SUCCESS;
}
