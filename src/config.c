#include "config.h"

void como_usar(void) {
	printf("Modo de uso:\n");
	printf("\n");
	printf("    matrix-mult [-a fil col -b fil col [-h hilos [-t part]] [-ni]]\n");
	printf("\n");
	printf("Opciones:\n");
	printf("    (sin opciones se imprime una multiplicación de ejemplo)\n");
	printf("\n");
	printf("    a fil col : cantidad de filas y columnas de la matriz A\n");
	printf("    b fil col : cantidad de filas y columnas de la matriz B\n");
	printf("    h hilos   : cantidad de hilos (0 por defecto)\n");
	printf("    t part    : tipo de particionamiento (1 por defecto)\n");
	printf("    ni        : no imprimir\n");
	printf("\n");
	printf("Argumentos:\n");
	printf("\n");
	printf("    fil   : entero positivo\n");
	printf("    col   : entero positivo\n");
	printf("    hilos : entero positivo (cuadrado perfecto si part es 2)\n");
	printf("    part  : 1 ó 2\n");
	
	exit(0);
}

void set_params(param_t *params, int argc, char **argv, bool *thread_count_read,
				bool *print_output) {
	
	bool condicion = false;
	bool matrix_a_sizes_read = false;
	bool matrix_b_sizes_read = false;
	bool distrib_type_read   = false;
	
	*thread_count_read = false;
	*print_output      = true;
	
	if (argc == 1) {
		// Ejemplo secuencial
		params->matrix_a_fil = SIZE_EXAMPLE;
		params->matrix_a_col = SIZE_EXAMPLE;
		params->matrix_b_fil = SIZE_EXAMPLE;
		params->matrix_b_col = SIZE_EXAMPLE;
		params->thread_count = 0;
		params->distrib_type = 1;
		
		matrix_a_sizes_read = true;
		matrix_b_sizes_read = true;
		*thread_count_read  = false;
		distrib_type_read   = false;
		
		condicion = true;
	}
	else if (argc >= MIN_ARGS_COUNT + 1 && argc <= MAX_ARGS_COUNT + 1) {
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
						
						if (params->matrix_a_fil == 0 || params->matrix_a_col == 0)
							matrix_a_sizes_read = false;
						else
							matrix_a_sizes_read = true;
					}
					else {
						params->matrix_b_fil = atoi(argv[i + 1]);
						params->matrix_b_col = atoi(argv[i + 2]);
						
						if (params->matrix_b_fil == 0 || params->matrix_b_col == 0)
							matrix_b_sizes_read = false;
						else
							matrix_b_sizes_read = true;
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
							matrix_a_sizes_read &&
							matrix_b_sizes_read;

				if (condicion) {
					params->thread_count = atoi(argv[i + 1]);
					
					if (params->thread_count == 0)
						(*thread_count_read) = false;
					else
						(*thread_count_read) = true;
					
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
							(*thread_count_read);

				if (condicion) {
					params->distrib_type = atoi(argv[i + 1]);
					distrib_type_read = true;
					
					// Avanzamos el indice
					i += 1;
					
					// Tipo de distribución debe ser 1d o 2d
					if (params->distrib_type != 1 && params->distrib_type != 2)
						condicion = false;
					
					/*
					 * Si tipo de distribución es 2d, la cantidad de
					 * hilos debe ser un cuadrado perfecto.
					 */
					if (params->distrib_type == 2 && 
							!es_cuadrado_perfecto(params->thread_count))
						condicion = false;
				}
			}
			else if (strcmp(argv[i], "-ni") == 0) {
				/*
				 * No imprimiremos las matrices como
				 * salida del programa.
				 */
				*print_output = false;
			}
			
			if (!condicion)
				break;
		}
	}
	
	/*
	 * Si la cantidad de argumentos o sus 
	 * valores no son correctos. Se especifica 
	 * la forma de utilizar el programa.
	 */	
	if (!condicion)
		como_usar();
}

void adjust_thread_count(param_t *params) {
	/*
	 * No crearemos una cantidad de hilos
	 * mayor que MAX_THREADS.
	 */
	if (params->thread_count > MAX_THREADS) {
		params->thread_count = MAX_THREADS;
		LOG(INFO, "Cantidad de hilos ajustada a %d", params->thread_count);
	}
	
	
	/*
	 * Verificaciones adicionales.
	 */
	bool condicion1 = (params->matrix_a_fil) < (params->thread_count);
	
	int thread_count_sqrt = (int) sqrt(params->thread_count);
	bool condicion2 = (params->matrix_a_fil) < (thread_count_sqrt) ||
					  (params->matrix_b_col) < (thread_count_sqrt);
	
	
	if (params->distrib_type == 1 && condicion1) {
		/*
		 * Cuando el particionamiento es 1d (una dimensión),
		 * la cantidad de hilos no puede ser menor a la cantidad
		 * de filas de la matriz (de salida) C.
		 * Asignamos una cantidad "uniforme" igual a la raíz
		 * cuadrada de la cantidad de filas (aplicando función
		 * piso).
		 */
		params->thread_count = (int) sqrt(params->matrix_a_fil);
		LOG(INFO, "Cantidad de hilos ajustada a %d", params->thread_count);
	}
	else if (params->distrib_type == 2 && condicion2) {
		/* 
		 * Análogamente, cuando el particionamiento es 2d 
		 * (dos dimensiones), la raíz cuadrada de la cantidad 
		 * de hilos no puede ser menor a la cantidad de filas
		 * o a la cantidad de columnas de la matriz (de
		 * salida) C.
		 * Asignamos una cantidad "uniforme" igual al cuadrado
		 * del máximo entre la raíz cuadrada de la cantidad de
		 * filas y la raíz cuadrada de la cantidad de columnas
		 * de la matriz (de salida) C. 
		 */
		int matrix_a_fil_sqrt = (int) sqrt(params->matrix_a_fil);
		int matrix_b_col_sqrt = (int) sqrt(params->matrix_b_col);
		
		params->thread_count = matrix_a_fil_sqrt > matrix_b_col_sqrt ?
							   matrix_a_fil_sqrt :
							   matrix_b_col_sqrt;
		
		params->thread_count *= params->thread_count;
		LOG(INFO, "Cantidad de hilos ajustada a %d", params->thread_count);
	}
}

void *matrix_mult_thread(void *args) {
	matrix_mult_args *aux = (matrix_mult_args *) args;
	
	matrix_mult(aux->matrix_a, aux->matrix_b, aux->matrix_c, 
				aux->row_begin, aux->row_count, aux->col_begin, aux->col_count);
	
	pthread_exit((void *) 0);
}

void distrib_1d(matrix_t *mat_a, matrix_t *mat_b, matrix_t *mat_c, 
		int thread_count, matrix_mult_args *arguments) {
	
	int i;
	
	// Realizamos la división del trabajo
	int rows_count     = matrix_rows(mat_c) / thread_count;
	int remainder_rows = matrix_rows(mat_c) % thread_count;
	
	// Distribuimos el trabajo
	for (i=0; i < thread_count; i++) {
		// A cada uno se les asigna las matrices
		arguments[i].matrix_a  = mat_a;
		arguments[i].matrix_b  = mat_b;
		arguments[i].matrix_c  = mat_c;
		
		// A cada uno se asigna rows_count filas
		arguments[i].row_begin = i * rows_count;
		arguments[i].row_count = rows_count;
		
		// A cada uno se asignan todas las columnas
		arguments[i].col_begin = 0;
		arguments[i].col_count = matrix_cols(mat_c);
	}
	
	/*
	 * Si sobraron filas sin asignar, éstas
	 * son asignadas al último hilo.
	 */
	if (remainder_rows > 0)
		arguments[thread_count - 1].row_count += remainder_rows;
}

void distrib_2d(matrix_t *mat_a, matrix_t *mat_b, matrix_t *mat_c, 
		int thread_count, matrix_mult_args *arguments) {
	
	int i, j, k;
	int thread_count_sqrt = (int) sqrt(thread_count);
	
	// Realizamos la división del trabajo
	int rows_count     = matrix_rows(mat_c) / thread_count_sqrt;
	int cols_count     = matrix_cols(mat_c) / thread_count_sqrt;
	int remainder_rows = matrix_rows(mat_c) % thread_count_sqrt;
	int remainder_cols = matrix_cols(mat_c) % thread_count_sqrt;

	// Distribuimos el trabajo
	k=0;
	for (i=0; i < thread_count_sqrt; i++) {
		for (j=0; j < thread_count_sqrt; j++) {
			// A cada uno se les asigna las matrices
			arguments[k].matrix_a  = mat_a;
			arguments[k].matrix_b  = mat_b;
			arguments[k].matrix_c  = mat_c;
			
			// A cada uno se asigna rows_count filas
			arguments[k].row_begin = i * rows_count;
			arguments[k].row_count = rows_count;
			
			// A cada uno se asignan todas las columnas
			arguments[k].col_begin = j * cols_count;
			arguments[k].col_count = cols_count;
			
			++k;
		}
		
		/*
		 * Si sobraron columnas sin asignar, éstas
		 * son asignadas al hilo en cuestión.
		 */
		if (remainder_cols > 0)
			arguments[k].col_count += remainder_cols;
	}
	
	/*
	 * Si sobraron filas sin asignar, éstas
	 * son asignadas a los hilos que recibieron
	 * las últimas filas.
	 */
	if (remainder_rows > 0) {
		k = thread_count - thread_count_sqrt;
		while (k < thread_count) {
			arguments[k].row_count += remainder_rows;
			++k;
		}
	}
}

void print_matrices(matrix_t *mat_a, matrix_t *mat_b, matrix_t *mat_c) {
	FILE *archivo = NULL;
	
	if ((archivo = fopen(OUTPUT_FILE, "w")) == NULL) {
		LOG(WARN, "Error al abrir archivo de matrices \"%s\". %s", 
				OUTPUT_FILE, "Las matrices no se imprimirán.");
		return;
	}
	
	fprintf(archivo, "Matriz A\n");
	matrix_print(mat_a, archivo);
	fprintf(archivo, "\n");
	fflush(archivo);

	fprintf(archivo, "Matriz B\n");
	matrix_print(mat_b, archivo);
	fprintf(archivo, "\n");
	fflush(archivo);

	fprintf(archivo, "Matriz C\n");
	matrix_print(mat_c, archivo);
	fprintf(archivo, "\n");
	fflush(archivo);
	
	fclose(archivo);
}

void print_times(time_rec_t tiempo_total_multip, time_rec_t tiempo_total_partit,
			time_rec_t tiempo_total_thr_creat, time_rec_t tiempo_total_thr_exec,
			int thread_count) {
	
	FILE *archivo = NULL;
	
	/*
	 * Impresión en la salida estándar
	 */
	fprintf(stdout, "Cantidad de Hilos (CH)...................%d\n", thread_count);
	
	fprintf(stdout, "Tiempo Total Multiplicación (TTM)........%lld\n", 
			TIME_DIFF(tiempo_total_multip));
	fprintf(stdout, "Tiempo Total Particionamiento (TTP)......%lld\n", 
			TIME_DIFF(tiempo_total_partit));
	fprintf(stdout, "Tiempo Total Creación Hilos (TTCH).......%lld\n", 
			TIME_DIFF(tiempo_total_thr_creat));
	fprintf(stdout, "Tiempo Total Ejecución Hilos (TTEH)......%lld\n", 
			TIME_DIFF(tiempo_total_thr_exec));

	fprintf(stdout, "Tiempo Promedio Creación Hilos (TPCH)....%f\n",
			TIME_DIFF(tiempo_total_thr_creat) / DOUBLE(thread_count));
	fprintf(stdout, "Tiempo Promedio Ejecución Hilos (TPEH)...%f\n",
			TIME_DIFF(tiempo_total_thr_exec) / DOUBLE(thread_count));

	/*
	 * Apertura de archivo
	 */
	if ((archivo = fopen(TIMES_FILE, "w")) == NULL) {
		LOG(WARN, "Error al abrir archivo de tiempos \"%s\". %s", 
				TIMES_FILE, "Las matrices no se imprimirán.");
		return;
	}
	
	/*
	 * Escritura en archivo.
	 */
	fprintf(archivo, "CH  \t%d\n", thread_count);
	fprintf(archivo, "TTM \t%lld\n", TIME_DIFF(tiempo_total_multip));
	fprintf(archivo, "TTP \t%lld\n", TIME_DIFF(tiempo_total_partit));
	fprintf(archivo, "TTCH\t%lld\n", TIME_DIFF(tiempo_total_thr_creat));
	fprintf(archivo, "TTEH\t%lld\n", TIME_DIFF(tiempo_total_thr_exec));
	
	fclose(archivo);
}

void print_partitions(matrix_mult_args *arguments, int thread_count) {
	FILE *archivo = NULL;
	int i;
	
	/*
	 * Apertura de archivo
	 */
	if ((archivo = fopen(PARTIT_FILE, "w")) == NULL) {
		LOG(WARN, "Error al abrir archivo de particiones \"%s\". %s", 
				PARTIT_FILE, "Las matrices no se imprimirán.");
		return;
	}
	
	fprintf(archivo, "Hilo,FilaIni,FilaCant,ColumIni,ColumCant\n");
	for (i=0; i < thread_count; i++) {
		fprintf(archivo, "%d,%d,%d,%d,%d\n", i, arguments[i].row_begin,
			arguments[i].row_count, arguments[i].col_begin, arguments[i].col_count);
	}
	
	fclose(archivo);
}
