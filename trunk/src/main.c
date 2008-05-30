#include "config.h"

/*
 * Función principal del programa.
 */
int main(int argc, char **argv) {
	matrix_t *mat_a, *mat_b, *mat_c;
	int i;
	param_t params = {0};
	bool thread_count_read = false;
	bool print_output = true;
	
	// Variables para control de tiempo
	time_rec_t tiempo_total_multip    = {0};
	time_rec_t tiempo_total_partit    = {0};
	time_rec_t tiempo_total_thr_creat = {0};
	time_rec_t tiempo_total_thr_exec  = {0};
	
	
	/*
	 * Verificamos si se pasó como argumento
	 * algún archivo de configuración, y 
	 * cargamos los parámentros de configuración.
	 */
	set_params(&params, argc, argv, &thread_count_read, &print_output);
	

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
	LOG(INFO, "Creando matrices.");
	matrix_create(&mat_a, params.matrix_a_fil, params.matrix_a_col);
	matrix_create(&mat_b, params.matrix_b_fil, params.matrix_b_col);
	matrix_create(&mat_c, matrix_rows(mat_a), matrix_cols(mat_b));
	
	/*
	 * Cargamos las matrices con valores
	 * aleatorios.
	 */
	matrix_fill(mat_a);
	matrix_fill(mat_b);
	
	
	// Inicio control de tiempo total de multiplicación.
	TIME_BEGIN(tiempo_total_multip);
	
	if (thread_count_read) {
		LOG(INFO, "Multiplicación concurrente con %d hilo(s).", params.thread_count);
		
		/*
		 * Multiplicación concurrentemente. La cantidad
		 * de hilos se debe ajustar apropiadamente.
		 */
		adjust_thread_count(&params);
		
		
		/*
		 * Realizar distribución de matrices
		 */
		LOG(INFO, "Realizando particionamiento de datos %dd.", params.distrib_type);
		
		// Inicio control de tiempo total de particionamiento.
		TIME_BEGIN(tiempo_total_partit);
		
		matrix_mult_args *arguments = GET_MEM(matrix_mult_args, params.thread_count);
		if (params.distrib_type == 1)
			distrib_1d(mat_a, mat_b, mat_c, params.thread_count, arguments);
		else if (params.distrib_type == 2)
			distrib_2d(mat_a, mat_b, mat_c, params.thread_count, arguments);
		else
			LOG(FATAL, "Particionamiento distinto a 1d y 2d");
		
		// Fin control de tiempo total de particionamiento.
		TIME_END(tiempo_total_partit);
		
		/*
		 * Creación de hilos
		 */
		LOG(INFO, "Creando hilos.");
		
		// Inicio control de tiempo total de ejecución de hilos.
		TIME_BEGIN(tiempo_total_thr_exec);
		
		// Inicio control de tiempo total de creación de hilos.
		TIME_BEGIN(tiempo_total_thr_creat);
		
		pthread_t *threads = GET_MEM(pthread_t, params.thread_count);
		for (i=0; i < params.thread_count; i++) {
			int rc = pthread_create(&threads[i], NULL, 
								matrix_mult_thread, &arguments[i]);
			
			if (rc != 0)
				LOG(FATAL, "Error en creación del hilo '%d'", i);
		}
		
		// Fin control de tiempo total de creación de hilos.
		TIME_END(tiempo_total_thr_creat);
		
		/*
		 * Esperar a los hilos.
		 */
		for (i=0; i < params.thread_count; i++) {
			int rc = pthread_join(threads[i], NULL);
			
			if (rc != 0)
				LOG(FATAL, "Error en 'join' del hilo '%d'", i);
		}
		
		// Fin control de tiempo total de ejecución de hilos.
		TIME_END(tiempo_total_thr_exec);
		
		/*
		 * Imprimimos las particiones de los hilos.
		 */
		print_partitions(arguments, params.thread_count);
		
		/*
		 * Realizar limpieza
		 */
		LOG(INFO, "Liberando memoria de hilos.");		
		free(threads);
		free(arguments);
	}
	else {
		/*
		 * Multiplicación secuencial.
		 */
		LOG(INFO, "Multiplicación secuencial.");
		matrix_mult(mat_a, mat_b, mat_c, 
					0, matrix_rows(mat_c), 
					0, matrix_cols(mat_c));
	}
	
	// Fin control de tiempo total de multiplicación.
	TIME_END(tiempo_total_multip);
	
	/*
	 * Imprimimos los tiempos obtenidos.
	 */
	printf("\n");
	print_times(tiempo_total_multip, 
				tiempo_total_partit, 
				tiempo_total_thr_creat, 
				tiempo_total_thr_exec,
				params.thread_count,
				mat_a, mat_b, mat_c);
	printf("\n");
	
	/*
	 * Imprimir las matrices
	 */
	if (print_output) {
		LOG(INFO, "Imprimiendo matrices.");
		print_matrices(mat_a, mat_b, mat_c);
	}
	
	
	/*
	 * Destruimos las matrices.
	 */
	LOG(INFO, "Liberando memoria de matrices.");
	matrix_destroy(mat_a);
	matrix_destroy(mat_b);
	matrix_destroy(mat_c);
	
	return EXIT_SUCCESS;
}
