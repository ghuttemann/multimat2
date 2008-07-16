#ifndef MATRIX_H_
#define MATRIX_H_


/*
 * Tipo de dato para los elementos
 * de las matrices.
 */
typedef float element_t;

/*
 * Formato de impresión de los
 * elementos de las matrices.
 */
#define ELEMENT_FORMAT "%f"

/*
 * Imprime un objeto del tipo matrix_t en
 * el archivo "output".
 */
void matrix_print(element_t *mat, int n, FILE *output);

/*
 * Carga una matriz con valores aleatorios.
 */
void matrix_fill(element_t *mat, int n);

/*
 * Calcula la traspuesta de una matriz
 */
void matrix_traspose(element_t *mat, int n);

/*
 * Multiplica dos matrices.
 */
void matrix_mult(element_t *a, element_t *b, element_t *c, int n,
				 int row_begin, int row_count, int col_begin, int col_count);

/*
 * Realiza el mapeo de indices de un arreglo
 * de dos dimensiones a un arreglo de una
 * dimensión.
 */
inline int matrix_map(int ncols, int i, int j);

/*
 * Obtiene el valor del elemento en la
 * posición (row, col) de un objeto del
 * tipo matrix_t.
 */
#define matrix_val(mat, ncols, row, col) (mat[matrix_map(ncols, row, col)])

/*
 * Obtiene la referencia del elemento en la
 * posición (row, col) de un objeto del
 * tipo matrix_t.
 */
#define matrix_ref(mat, ncols, row, col) (*(mat + matrix_map(ncols, row, col)))

#endif /*MATRIX_H_*/
