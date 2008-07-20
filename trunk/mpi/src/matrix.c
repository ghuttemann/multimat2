#include "utils.h"
#include "matrix.h"

void matrix_print(element_t *mat, int n, int m, FILE *output) {
    int i, j;

    for (i=0; i < n; i++) {
        for (j=0; j < m; j++) {
            fprintf(output, ELEMENT_FORMAT, mat[matrix_map(m, i, j)]);
            fprintf(output, "\t");
        }
        fprintf(output, "\n");
    }
}

void matrix_fill(element_t *mat, int tam) {
	int i;
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
	for (i=0; i < tam; i++)
        mat[i] = (element_t) (10.0 * (rand() / (RAND_MAX + 1.0)));
}

void matrix_clear(element_t *mat, int tam) {
	int i;
    
	for (i=0; i < tam; i++)
        mat[i] = 0;
}

void matrix_traspose(element_t *mat, int n) {
	int i, j;
	element_t tmp;
	
	for (i=0; i < n; i++)
	for (j=0; j < n; j++)
		if (i != j) {
			tmp = mat[matrix_map(n, i, j)];
			mat[matrix_map(n, i, j)] = mat[matrix_map(n, j, i)];
			mat[matrix_map(n, j, i)] = tmp;
		}
}

void matrix_mult(element_t *a, element_t *b, element_t *c, int n) {
	int i, j, k;

	for (i=0; i < n; i++)
	for (j=0; j < n; j++)
	for (k=0; k < n; k++)
		c[matrix_map(n, i, j)] += a[matrix_map(n, i, k)] * b[matrix_map(n, k, j)];
}

void matrix_load(element_t *mat, int n, matrix_name_t mat_name) {
    switch (mat_name) {
        case A:
        case B:
            matrix_fill(mat, n * n);
            break;
            
        case C:
            matrix_clear(mat, n * n);
            break;
    }
}
