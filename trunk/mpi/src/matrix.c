#include "utils.h"
#include "matrix.h"

void matrix_print(element_t *mat, int n, FILE *output) {
    int i, j;

    for (i=0; i < n; i++) {
        for (j=0; j < n; j++) {
            fprintf(output, ELEMENT_FORMAT, matrix_val(mat, n, i, j));
            fprintf(output, "\t");
        }
        
        fprintf(output, "\n");
    }
}

void matrix_fill(element_t *mat, int n) {
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
	for (i=0; i < n; i++)
    for (j=0; j < n; j++)
        matrix_ref(mat, n, i, j) = (element_t) (10.0 * (rand() / (RAND_MAX + 1.0)));
}

void matrix_traspose(element_t *mat, int n) {
	int i, j;
	element_t tmp;
	
	for (i=0; i < n; i++)
	for (j=0; j < n; j++)
		if (i != j) {
			tmp = matrix_val(mat, n, i, j);
			matrix_ref(mat, n, i, j) = matrix_val(mat, n, j, i);
			matrix_ref(mat, n, j, i) = tmp;
		}
}

void matrix_mult(element_t *a, element_t *b, element_t *c, int n,
				 int row_begin, int row_count, int col_begin, int col_count) {

	int i=0, j=0, k=0, k_end=0;

	k_end = n - 1;

	for (i=row_begin; i < (row_begin + row_count); i++)
	for (j=col_begin; j < (col_begin + col_count); j++)
	for (k=0; k <= k_end; k++)
		matrix_ref(c, n, i, j) += matrix_val(a, n, i, k) * matrix_val(b, n, k, j);
}

inline int matrix_map(int ncols, int i, int j) {
	return (ncols * i + j);
}
