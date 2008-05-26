#include "matrix.h"

void test_simple();
void test_mult();

int main_test(int argc, char** argv) {
    test_simple();
    test_mult();
    return (EXIT_SUCCESS);
}

void test_simple() {
    matrix_t *mat;
    int i=0, j=0;
    
    printf("\ntest_simple\n");
    
    matrix_create(&mat, 2, 2);
    matrix_print(mat, stdout);
    
    srand(time(NULL));
    for (i=0; i < matrix_rows(mat); i++)
    for (j=0; j < matrix_cols(mat); j++)
        matrix_ref(mat, i, j) = rand() / 1333.33;
    
    printf("\n");
    matrix_print(mat, stdout);
    matrix_destroy(mat);
}

void test_mult() {
	matrix_t *a, *b, *c;
	
	printf("\ntest_mult\n");
	    
    // Create matrices A, B, C
	matrix_create(&a, 3, 4);
    matrix_create(&b, 4, 2);
    matrix_create(&c, 3, 2);
    
    // Fill matrix A
    matrix_ref(a, 0, 0) = 3;
    matrix_ref(a, 0, 1) = 5;
    matrix_ref(a, 0, 2) = 8;
	matrix_ref(a, 0, 3) = 4;
	matrix_ref(a, 1, 0) = 2;
	matrix_ref(a, 1, 1) = 1;
	matrix_ref(a, 1, 2) = 0;
	matrix_ref(a, 1, 3) = 2;
	matrix_ref(a, 2, 0) = 3;
	matrix_ref(a, 2, 1) = 5;
	matrix_ref(a, 2, 2) = 7;
	matrix_ref(a, 2, 3) = 2;
    
    // Fill matrix B
    matrix_ref(b, 0, 0) = 1;
    matrix_ref(b, 0, 1) = 2;
    matrix_ref(b, 1, 0) = 7;
    matrix_ref(b, 1, 1) = 3;
    matrix_ref(b, 2, 0) = 2;
    matrix_ref(b, 2, 1) = 1;
    matrix_ref(b, 3, 0) = 4;
    matrix_ref(b, 3, 1) = 3;
    
    // Print matrices A, B
    matrix_print(a, stdout);
    printf("\n");
    matrix_print(b, stdout);
    
    // Multiply matrices A, B
    matrix_mult(a, b, c, 0, 0, 0, 1);
    matrix_mult(a, b, c, 1, 1, 0, 1);
    matrix_mult(a, b, c, 2, 2, 0, 1);
    
    // Print matrix C
    printf("\n");
    matrix_print(c, stdout);
    
    // Destroy matrices A, B, C
    matrix_destroy(a);
    matrix_destroy(b);
    matrix_destroy(c);
}

