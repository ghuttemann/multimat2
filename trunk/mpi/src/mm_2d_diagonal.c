/* 
 * File:   mm_2d_diagonal.c
 * Author: mrodas
 *
 * Created on 17 de julio de 2008, 08:01 PM
 */

#include "utils.h"
#include "matrix.h"

/******************************************************************************
 ************************** D E F I N I C I O N E S ***************************
 ******************************************************************************/

#define MPI_ELEMENT_T MPI_FLOAT

#define ROW 0
#define COL 1

int get_matrix_size(int, char **);
void diagonal_matrix_multiply(int, element_t *, element_t *, element_t *, MPI_Comm);
void soloPasa_proceso_0(int);
void liberar_procesos(int, int);
    
/******************************************************************************
 ********************* F U N C I O N  P R I N C I P A L ***********************
 ******************************************************************************/
int main(int argc, char** argv) {
    int myRank   = -1; 	// Posicion dentro del comunicador
    int commSize = 0;	// Tamaño del comunicador
    int matSize  = 0; 	// Tamaño de la matriz
    int blkSize  = 0; 	// Tamaño del bloque
    bool printMatrix;   // Bandera para impresion de matrices
    
    MPI_Log(INFO, "Antes de MPI_Init");

    /*
     * Inicializamos el entorno MPI.
     */
    MPI_Init(&argc, &argv);
    MPI_Log(INFO, "Entorno MPI inicializado");

    /*
     * Obtenemos el tamaño del comunicador.
     */
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    MPI_Log(INFO, "Cantidad procesos: %d", commSize);

    /*
	 * Obtenemos el ranking del proceso e 
     * inicializamos la semilla para la
     * generación de números aleatorios.
	 */
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    srand(myRank);
    
    // Obtener tamaño de la matriz
    if ((matSize = get_matrix_size(argc, argv)) < 0) {
        if (myRank != 0)
            MPI_Exit(1);
        else {
            MPI_Log(FATAL, "%s %s (%s '%s').",
                    "Dimension de matriz debe ser un numero",
                    "entero, positivo y cuadrado perfecto",
                    "se recibio", argv[1]);
        }
    }
    
    /*
	 * Verificamos que el tamaño del comunicador
	 * sea adecuado.
	 */
    if (!is_perfect_square(commSize)) {
        if (myRank != 0)
            MPI_Exit(1);
        else {
            MPI_Log(FATAL, "Tamaño del comunicador (%d) "
                    "debe ser cuadrado perfecto", commSize);
        }
    }
    
    /*
     * Ademas, la dimension de las matrices debe
     * ser divisible entre la raiz cuadrada del
     * tamaño del comunicador (cantidad de procesos).
     */
    int commSizeSqrt = (int) sqrt(commSize);
    
    if (matSize % commSizeSqrt != 0) {
        if (myRank != 0)
            MPI_Exit(1);
        else {
            MPI_Log(FATAL, "Dimension de matriz (%d) debe ser divisible "
                    "entre raiz cuadrada del tamaÃ±o del comunicador (%d)", 
                    matSize, 
                    commSizeSqrt);
        }
    }
    
    /*
     * Establecemos la bandera que indica si se
     * imprimir o no las matrices.
     */
    if (argc >= 3 && strcmp("p", argv[2]) == 0)
        printMatrix = true;
    else
        printMatrix = false;
    
    /*
     * Calculamos el tamaño de bloque.
     */
    blkSize = matSize / commSizeSqrt;
    
    /**************************************************************************
     * Construimos la topología 2D
     */
    int dims[2], periods[2], myCoords[2], myRank_2d;
    MPI_Comm comm_2d;
    
    /* Se calcula variables del grid de procesos */
    dims[ROW] = dims[COL] = (int) sqrt(commSize);
    
    /* 
     * Se crea la topologia cartesiana sobre la matriz de procesos.
     */
    periods[ROW] = periods[COL] = 0; // Establece como no circular

    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &comm_2d);
    MPI_Comm_rank(comm_2d, &myRank_2d);
    MPI_Cart_coords(comm_2d, myRank_2d, 2, myCoords);
    /**************************************************************************/
    /*
     * Inicializacion de la Matriz A, B y C.
     */
    element_t *matA = GET_MEM(element_t, blkSize * matSize);
    element_t *matB = GET_MEM(element_t, blkSize * matSize);
    element_t *matC = GET_MEM(element_t, blkSize * matSize);

    // Rellenar matrices A y B
    if (myCoords[ROW] == myCoords[COL]) {
        matrix_fill(matA, blkSize * matSize);
        matrix_fill(matB, blkSize * matSize);
    }

    // Cerar matriz C
    if (myCoords[ROW] == myCoords[COL])
        matrix_clear(matC, blkSize * matSize);
    
    MPI_Debug(INFO, "Matrices A, B y C ya se crearon");

    if (printMatrix) {
        MPI_Barrier(MPI_COMM_WORLD);
        soloPasa_proceso_0(myRank);
        
        if (myCoords[ROW] == myCoords[COL]) {
            matrix_print(matA, matSize, blkSize, stdout);
            printf("[%d, %d]TERMINA A \n",  myCoords[ROW], myCoords[COL]);
            matrix_print(matB, blkSize, matSize, stdout);
            printf("[%d, %d]TERMINA B \n",  myCoords[ROW], myCoords[COL]);
            MPI_Log(INFO, "--------------------------------------------------");
        }
        
        liberar_procesos(myRank, commSize);
        MPI_Barrier(MPI_COMM_WORLD);
    }
    
  	/*
  	 *  Proceso Principal del Algoritmo 2-D Diagonal
  	 */
    // Inicio control tiempo.
    double initTime = MPI_Wtime();
    
    diagonal_matrix_multiply(matSize, matA, matB, matC, comm_2d);
    
    // Fin control tiempo.
    double endTime = MPI_Wtime();

    MPI_Log(INFO, "FIN DEL PROCESO");
    
    /*
     * Impresion de las matrices.
     */
    if (printMatrix) {
        MPI_Barrier(MPI_COMM_WORLD);
        soloPasa_proceso_0(myRank);

        if (myCoords[ROW] == myCoords[COL]) {
            matrix_print(matC, matSize, blkSize, stdout);
            printf("[%d, %d]TERMINA C \n", myCoords[ROW], myCoords[COL]);
        }
        
        liberar_procesos(myRank, commSize);
        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    // Liberamos el comunicador 2D
    MPI_Comm_free(&comm_2d);
    
    // Liberar buffers
    free(matA);
    free(matB);
    free(matC);
    
    // Impresión de tiempos.
    print_parallel_time(initTime, endTime, myRank);
    
    // Termina MPI.
    MPI_Exit(EXIT_SUCCESS);
}

/******************************************************************************
 ***************************** F U N C I O N E S ******************************
 ******************************************************************************/

/*
 * Obtiene el tamaño de la matriz desde la línea de argumento. Si el
 * parámetro es inválido retorna un número negativo.
 */
int get_matrix_size(int argc, char *argv[]) {
    
	if (argc >= 2 && is_positive_integer(argv[1]))
        return atoi(argv[1]);
    
    return -1;
}

/*
 * Solo pasa el proceso 0. El resto se bloque en espera del proceso 0;
 */
void soloPasa_proceso_0(int myRank) {
    int b[1] = {1};
    MPI_Status status;
    
    if(myRank > 0) {
        MPI_Recv(b, 1, MPI_INT, myRank - 1, 1, MPI_COMM_WORLD, &status);
    }
}

/*
 * Libera a la siguiente matriz
 */
void liberar_procesos(int myRank, int commSize) {
    int b[1] = {1};
    
    if (myRank < commSize - 1) {
        MPI_Send(b, 1, MPI_INT, myRank+1, 1, MPI_COMM_WORLD);
    }
}

/*
 * Calcula la multiplicacion para una matriz A y un vector B
 * dando como resultado el vector C
 */
void diagonal_matrix_multiply(int N, element_t *A, element_t *B, element_t *C, 
                                MPI_Comm comm_2d) {
    
    int i, j, k;
    int nlocal, keep_dims[2];
    int myCoords[2], comm2dSize;
    int rootRank, coords[2], myRank_2d;
    MPI_Comm comm_row, comm_col;

    // Tamaño del comunicador.
    MPI_Comm_size(comm_2d, &comm2dSize);
    
    // Obtenemos rank y coordenada
    MPI_Comm_rank(comm_2d, &myRank_2d);
    MPI_Cart_coords(comm_2d, myRank_2d, 2, myCoords);
    
    // Tamaño del bloque.
    nlocal = N / ((int) sqrt(comm2dSize));
    
    /* Se crea la sub-topologia con base en las filas */
    keep_dims[ROW] = 0;
    keep_dims[COL] = 1;
    MPI_Cart_sub(comm_2d, keep_dims, &comm_row);
    
    MPI_Debug(INFO, "Subgrilla de Filas creada por P[%d, %d]", 
            myCoords[ROW], myCoords[COL]);

    /* Se crea la sub-topologia con base en las columnas */
    keep_dims[ROW] = 1;
    keep_dims[COL] = 0;
    MPI_Cart_sub(comm_2d, keep_dims, &comm_col);
    
    MPI_Debug(INFO, "Subgrilla de Columnas creada por P[%d, %d]", 
            myCoords[ROW], myCoords[COL]);
    
    /*
     * -> Paso 1: Distribuir los elementos desde la Diagonal en el eje "y"
     */    
    coords[ROW] = myCoords[COL];
    MPI_Cart_rank(comm_col, coords, &rootRank);
    
    /*
     * Operacion 1: Broadcast de columnas
     * Broadcast de las columnas de A (A*,j) a los procesos en la columna j
     * del mesh de procesos (p*,j).
     */
    MPI_Bcast(A, N * nlocal, MPI_ELEMENT_T, rootRank, comm_col);
    MPI_Debug(INFO, "<%d, %d> PASO 1 (BCAST)", myCoords[ROW], myCoords[COL]);

    /*
     * Construir mensaje para el Scatter.
     */
    element_t *Baux = GET_MEM(element_t, N * nlocal);
    int pos = 0;
    
    for (k=0; k < N; k += nlocal)
        for (i=0; i < nlocal; i++)
            for (j=k; j < k + nlocal; j++)
                Baux[pos++] = B[matrix_map(N, i, j)];
    
    /*
     * Operacion 2: Scatter de filas de B
     * One-to-all personalized broadcast de los elementos de la fila
     * B (Bi,*) a los procesos en la columna j del mesh de procesos
     * pj,*)
     */
    MPI_Scatter(Baux, nlocal * nlocal, MPI_ELEMENT_T, B, nlocal * nlocal, 
                MPI_ELEMENT_T, rootRank, comm_col);
    MPI_Debug(INFO, "<%d, %d> PASO 1 (SCATTER)", myCoords[ROW], myCoords[COL]);

    /*
     * Multiplicación de matrices.
     * Se multiplica A por una parte de B y se guarda en px.
     */
    MPI_Debug(INFO, "Inicio de multiplicacion en P[%d, %d]", 
            myCoords[ROW], myCoords[COL]);
    
    // Se reserva memoria para la multiplicación parcial
    element_t *px = GET_MEM(element_t, nlocal * N);
    matrix_clear(px, nlocal * N);
    
    for (i=0; i < N; i++) {
        for (j=0; j < nlocal; j++) {
            for (k=0; k < nlocal; k++) {
                px[matrix_map(nlocal, i, j)] += A[matrix_map(nlocal, i, k)] * 
                                                B[matrix_map(nlocal, k, j)];
            }
        }
    }
    
    MPI_Debug(INFO, "Fin de multiplicacion en P[%d, %d]", 
            myCoords[ROW], myCoords[COL]);
    
    /*
     * -> Paso 2: Enviamos el Resultado en la direccion "x".
     *    -> Operacion: Reduce de filas
     *    All-to-One reduction de los elementos de la fila de
     *    B (Bi,*) a los procesos en la columna j del mesh de procesos
     *    pj,*
     */
    coords[ROW] = myCoords[ROW];
    
    MPI_Cart_rank(comm_row, coords, &rootRank);
    MPI_Reduce(px, C, nlocal * N, MPI_ELEMENT_T, MPI_SUM, rootRank, comm_row);
    
    MPI_Debug(INFO, "<%d, %d> PASO 2 (REDUCE)=> %d", 
            myCoords[ROW], myCoords[COL], (nlocal * N));
    
    // Se liberan los comunicadores creados.
    MPI_Comm_free(&comm_row);
    MPI_Comm_free(&comm_col);
    
    // Se liberan variables auxiliares.
    free(px);
    free(Baux);
}
