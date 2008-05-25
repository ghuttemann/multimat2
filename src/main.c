#include "config.h"

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
