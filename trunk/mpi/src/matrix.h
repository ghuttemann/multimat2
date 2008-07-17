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
 * Pone a cero todos los elementos de la matriz.
 */
void matrix_clear(element_t *mat, int n);

/*
 * Calcula la traspuesta de una matriz
 */
void matrix_traspose(element_t *mat, int n);

/*
 * Multiplica dos matrices cuadradas.
 */
void matrix_mult(element_t *a, element_t *b, element_t *c, int n);

/*
 * Realiza el mapeo de indices de un arreglo
 * de dos dimensiones a un arreglo de una
 * dimensión.
 */
#define matrix_map(ncols, fil, col) (ncols * fil + col)

#endif /*MATRIX_H_*/
