#ifndef MATRIX_H_
#define MATRIX_H_

#include "utils.h"

/*
 * Tipo de dato para los
 * elementos de una matriz.
 */
#ifdef FLOAT
    #define matrix_elem_t float
    #define MATRIX_ELEM_T_FORMAT "%f"
#else
    #define matrix_elem_t unsigned int
    #define MATRIX_ELEM_T_FORMAT "%d"
#endif

/*
 * Tipo de dato matriz.
 */
typedef struct {
    matrix_elem_t **elements;
    int rows;
    int cols;
} matrix_t;

/*
 * Tipo de dato para pasar los
 * argumentos a la función de
 * multiplicación.
 */
typedef struct {
	matrix_t *matrix_a;
	matrix_t *matrix_b;
	matrix_t *matrix_c;
	int row_begin;
	int row_count;
	int col_begin;
	int col_count;
} matrix_mult_args;

/*
 * Crea una objeto del tipo matrix_t con nrows
 * filas y ncols columnas, e inicializa todos
 * los elementos a cero.
 */
void matrix_create(matrix_t **mat, int nrows, int ncols);

/*
 * Destruye un objeto del tipo matrix_t.
 */
void matrix_destroy(matrix_t *mat);

/*
 * Imprime un objeto del tipo matrix_t en
 * el archivo "destino".
 */
void matrix_print(matrix_t *mat, FILE *destino);

/*
 * Carga una matriz con valores aleatorios.
 */
void matrix_fill(matrix_t *mat);

/*
 * Multiplica dos matrices.
 */
void matrix_mult(matrix_t *a, matrix_t *b, matrix_t *c,
				 int row_begin, int row_count, int col_begin, int col_count);

/*
 * Obtiene el numero de filas de un objeto 
 * del tipo matrix_t.
 */
#define matrix_rows(mat) mat->rows

/* 
 * Obtiene el numero de columnas de un objeto 
 * del tipo matrix_t.
 */
#define matrix_cols(mat) mat->cols

/*
 * Obtiene el valor del elemento en la 
 * posición (row, col) de un objeto del 
 * tipo matrix_t.
 */
#define matrix_val(mat, row, col) (mat->elements[row][col])

/*
 * Obtiene la referencia del elemento en la 
 * posición (row, col) de un objeto del 
 * tipo matrix_t.
 */
#define matrix_ref(mat, row, col) (*(mat->elements[row] + col))

#endif /*MATRIX_H_*/
