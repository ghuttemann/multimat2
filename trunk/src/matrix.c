#include "matrix.h"

void matrix_create(matrix_t **mat, int nrows, int ncols) {
    int i, j;
    
    // Chequeo de rangos
    if (nrows <= 0 || ncols <= 0)
        LOG(FATAL, "%s: %s", __func__, "El número de filas y/o columnas debe ser positivo.");
    
    // Asignación de memoria para matrix_t
    (*mat) = GET_MEM(matrix_t, 1);
    
    // Asignación de memoria para los elementos de la matriz
    (*mat)->elements = GET_MEM(matrix_elem_type *, nrows);
    for (i=0; i < nrows; i++)
        (*mat)->elements[i] = GET_MEM(matrix_elem_type, ncols);
    
    // Establecer el número de filas y columnas
    (*mat)->rows = nrows;
    (*mat)->cols = ncols;
    
    // Inicialización de los elementos a cero
    for (i=0; i < (*mat)->rows; i++)
    for (j=0; j < (*mat)->cols; j++)
        (*mat)->elements[i][j] = 0;
}

void matrix_destroy(matrix_t *mat) {
    int i;
    
    // Liberar cada fila de la matriz
    for (i=0; i < mat->rows; i++)
        free(mat->elements[i]);
    
    // Liberar la matriz
    free(mat->elements);
    
    // Liberar el objeto matrix_t
    free(mat);
}

void matrix_print(matrix_t *mat, FILE *destino) {
    int i, j;
    
    for (i=0; i < matrix_rows(mat); i++) {
        for (j=0; j < matrix_cols(mat); j++) {
            fprintf(destino, "%d\t", matrix_val(mat, i, j));
        }
        fprintf(destino, "\n");
    }
}

void matrix_fill(matrix_t *mat) {
	//const static int divisor = MATRIX_ELEM_TYPE_MAX - MATRIX_ELEM_TYPE_MIN + 1;
	int i, j;
	static bool first_time = true;
	
	/*
	 * Solo inicializamos la semilla
	 * en la primera llamada.
	 */
	if (first_time) {
		srand(time(NULL));
		first_time = false;
	}
    
	/*
	 * Cargamos la matriz con digitos decimales
	 */
	for (i=0; i < matrix_rows(mat); i++)
    for (j=0; j < matrix_cols(mat); j++)
        matrix_ref(mat, i, j) = (matrix_elem_type) (10.0 * (rand() / (RAND_MAX + 1.0)));
}

void matrix_mult(matrix_t *a, matrix_t *b, matrix_t *c,
				 int row_begin, int row_count, int col_begin, int col_count) {
	
	int i=0, j=0, k=0, k_end=0;
	
	if (matrix_cols(a) != matrix_rows(b))
		LOG(FATAL, "%s: %s %s", __func__, 
				"El número de columnas de la matriz A debe ser igual a",
				"el númbero de filas de la matriz B");
	
	k_end = matrix_cols(a) - 1;
	
	for (i=row_begin; i < (row_begin + row_count); i++)
	for (j=col_begin; j < (col_begin + col_count); j++)
	for (k=0; k <= k_end; k++)
		matrix_ref(c, i, j) += matrix_val(a, i, k) * matrix_val(b, k, j);
}
