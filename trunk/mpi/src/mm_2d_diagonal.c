/* 
 * File:   mm_2d_diagonal.c
 * Author: mrodas
 *
 * Created on 17 de julio de 2008, 08:01 PM
 */

#include <mpi.h>
#include "utils.h"
#include "matrix.h"

/******************************************************************************
 ************************** D E F I N I C I O N E S ***************************
 ******************************************************************************/

#define MPI_ELEMENT_T MPI_FLOAT

/*
 * Tipo de dato que representa
 * una coordenada en 2D.
 */
typedef struct {
    int fil, col;
} coord_2d;

int get_matrix_size(int, char **);
void diagonal_matrix_multiply(int, element_t *, element_t *, element_t *, MPI_Comm);

/******************************************************************************
 ********************* F U N C I O N  P R I N C I P A L ***********************
 ******************************************************************************/
int main(int argc, char** argv) {
    int myRank   = -1; 	// Posicion dentro del comunicador
    int commSize = 0;	// TamaÃ±o del comunicador
    int matSize  = 0; 	// TamaÃ±o de la matriz
    int blkSize  = 0; 	// TamaÃ±o del bloque
    bool printMatrix;   // Bandera para impresión de matrices
    
    MPI_Log(INFO, "Antes de MPI_Init\n");

    /*
     * Inicializamos el entorno MPI.
     */
    MPI_Init(&argc, &argv);
    MPI_Log(INFO, "Entorno MPI inicializado\n");

    /*
     * Obtenemos el tamaÃ±o del comunicador.
     */
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    MPI_Log(INFO, "Cantidad procesos: %d \n", commSize);

    /*
     * Obtenemos el ranking del proceso.
     */
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    
    // Obtener tamaÃ±o de la matriz
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
	 * Verificamos que el tamaÃ±o del comunicador
	 * sea adecuado.
	 */
    if (!is_perfect_square(commSize)) {
        if (myRank != 0)
            MPI_Exit(1);
        else {
            MPI_Log(FATAL, "TamaÃ±o del comunicador (%d) "
                    "debe ser cuadrado perfecto", commSize);
        }
    }
    
    /*
     * Además, la dimensión de las matrices debe
     * ser divisible entre la raíz cuadrada del
     * tamaño del comunicador (cantidad de procesos).
     */
    int commSizeSqrt = (int) sqrt(commSize);
    
    if (matSize % commSizeSqrt != 0) {
        if (myRank != 0)
            MPI_Exit(1);
        else {
            MPI_Log(FATAL, "Dimension de matriz (%d) debe ser divisible "
                    "entre raiz cuadrada del tamaño del comunicador (%d)", 
                    matSize, 
                    commSizeSqrt);
        }
    }
    
    /*
     * Establecemos la bandera que indica si se
     * imprimirán o no las matrices.
     */
    if (argc >= 3 && strcmp("p", argv[2]) == 0)
        printMatrix = true;
    else
        printMatrix = false;
    
    /*
     * Calculamos el tamaño de bloque.
     */
    blkSize = matSize / commSizeSqrt;
   
    /*
     * Inicializacion de la Matriz A, B y C.
     */
    element_t *matA = GET_MEM(element_t, blkSize * matSize);
    element_t *matB = GET_MEM(element_t, blkSize * matSize);
    element_t *matC = GET_MEM(element_t, blkSize * blkSize);

    // Rellenar matrices A y B
    matrix_fill(matA, blkSize);
    matrix_fill(matB, blkSize);

    // Cerar matriz C
    matrix_clear(matC, matSize);
    MPI_Log(INFO, "Matrices A, B y C ya se crearon \n");
    
  	/************************************************************************
  	 *  Proceso Principal del Algoritmo 2-D Diagonal
  	 ************************************************************************/
    // Inicio control tiempo.
    double initTime = MPI_Wtime();
    
    diagonal_matrix_multiply(matSize, matA, matB, matC, MPI_COMM_WORLD);
    
    // Fin control tiempo.
    double endTime = MPI_Wtime();
    /************************************************************************
     *  Fin del Proceso Principal 
     ************************************************************************/    
    /*
     * Impresión de las matrices.
     */
    MPI_Log(INFO, "FIN DEL PROCESO. FALTA IMPRIMIR MATRICES \n");
    if (printMatrix) {
        matrix_print(matA, blkSize, stdout);
        printf("\n");
        matrix_print(matB, blkSize, stdout);
        printf("\n");
        matrix_print(matC, blkSize, stdout);
    }
    
    // Liberar buffers
    free(matA);
    free(matB);
    free(matC);
    
    if (myRank == 0)
        print_parallel_time(initTime, endTime);

    return (EXIT_SUCCESS);
}

/******************************************************************************
 ***************************** F U N C I O N E S ******************************
 ******************************************************************************/

/*
 * Obtiene el tamaÃ±o de la matriz desde la linea de argumento. Si el
 * parametro es invalido retorna un numero negativo.
 */
int get_matrix_size(int argc, char *argv[]) {
    int tmp;
    
	if (argc >= 2 && is_positive_integer(argv[1]) && 
        is_perfect_square(tmp = atoi(argv[1]))) {

		return tmp;
	}
    
    return -1;
}

/*
 * Calcula la multiplicaciÃ³n para una matriz a y un vector b
 * dando como resultado el vector x
 */
void diagonal_matrix_multiply(int N, element_t *A, element_t *B, element_t *C, 
                                MPI_Comm comm) {
    
    int ROW = 0, COL = 1;
    int i, j, nlocal;
    int npes, dims[2], periods[2], keep_dims[2];
    int myrank, my2drank, mycoords[2];
    int other_rank, coords[2];
    MPI_Status status;
    MPI_Comm comm_2d, comm_row, comm_col;

    /* Se obtiene información del comunicador */
    MPI_Comm_size(comm, &npes);
    MPI_Comm_rank(comm, &myrank);

    /* Se calcula el tamaña del grip de procesos */
    dims[ROW] = dims[COL] = sqrt(npes);
    nlocal = N / dims[ROW];

    /* Se reserva memoria para la multiplicación parcial */
    element_t * px = GET_MEM(element_t, N*N);
    matrix_clear(px, N);

    MPI_Log(INFO, "#%d# TOPOLOGIA VALORES:,%d ,%d, %d", myrank, nlocal,N,dims[ROW]);
    /* 
     * Se crea la topología cartesian sobre la matriz de procesos.
     */
    periods[ROW] = periods[COL] = 0; /* Establece como no circular */

    MPI_Cart_create(comm, 2, dims, periods, 1, &comm_2d);
    MPI_Comm_rank(comm_2d, &my2drank);
    MPI_Cart_coords(comm_2d, my2drank, 2, mycoords);

    /* Se crea la sub-topología con base en las columnas */
    keep_dims[ROW] = 0;
    keep_dims[COL] = 1;
    MPI_Cart_sub(comm_2d, keep_dims, &comm_row);

    /* Create the column-based sub-topology */
    keep_dims[ROW] = 1;
    keep_dims[COL] = 0;
    MPI_Cart_sub(comm_2d, keep_dims, &comm_col);

    /*
     * INICIALIZAR MATRIZ
     *   -> Poner en los elementos de la diagonal las columnas y filas
     *      correspondientes     
     */
    MPI_Datatype bloqueA;
    MPI_Datatype bloqueB;
    MPI_Type_vector(N,dims[COL],N,MPI_ELEMENT_T,&bloqueA);
    MPI_Type_commit(&bloqueA);
    MPI_Type_vector(dims[ROW],N,N,MPI_ELEMENT_T,&bloqueB);
    MPI_Type_commit(&bloqueB);
    
    MPI_Log(INFO, "#%d# INICIALIZACION DE MATRICES (SE ENVIA) \n", myrank);
    
    // Se envia a la diagonal
    if (mycoords[COL] == 0 && mycoords[ROW] != 0) {
        // Se inicializan las variables y bloques
        coords[ROW] = mycoords[ROW];
        coords[COL] = mycoords[ROW];
        
        // Se localiza las coordenadas de las diagonales
        MPI_Cart_rank(comm_2d, coords, &other_rank);
        /*
         * Se envian Columnas de A y Filas de B a sus correspondientes 
         * elementos de procesamiento (pij) de las diagonales.
         */
        MPI_Send(&(A[nlocal*coords[COL]]), N*nlocal, bloqueA, other_rank, 1, comm_2d);
        MPI_Send(&(B[nlocal*coords[ROW]]), N*nlocal, bloqueB, other_rank, 1, comm_2d);
    }
    
    MPI_Log(INFO, "#%d# INICIALIZACION DE MATRICES (SE RECIBE) \n", myrank);
    // Se recibe en la diagonal
    if (mycoords[ROW] == mycoords[COL] && mycoords[ROW] != 0) {
        coords[ROW] = mycoords[ROW];
        coords[COL] = 0;
        MPI_Cart_rank(comm_2d, coords, &other_rank);
        MPI_Recv(&(A[nlocal*coords[COL]]), N*nlocal, bloqueA, other_rank, 1, comm_2d,
                &status);
        MPI_Recv(&(B[nlocal*coords[ROW]]), N*nlocal, bloqueB, other_rank, 1, comm_2d,
                &status);
    }
    
    int val=MPI_Barrier(comm_2d);
    MPI_Log(INFO, "<%d> BARRIER (MAIN) \n", val);    

    /*
     * Paso 1: -> Distribuir los elementos desde la Diagonal en el eje x  
     */    
    coords[COL] = mycoords[COL];
    coords[ROW] = mycoords[COL];
    MPI_Cart_rank(comm_col, coords, &other_rank);
    /*
     * Operacion: Broadcast de columnas
     * Broadcast de las columnas de A (A*,j) a los procesos en la columna j
     * del mesh de procesos (p*,j).
     */
    MPI_Log(INFO, "<%d> PASO 1 (BCAST) \n", myrank);
    MPI_Bcast(&(A[nlocal*coords[COL]]), N*nlocal, bloqueA, other_rank, comm_col);

    /*
     * Operacion: Scatter de filas
     * One-to-all personalized broadcast de los elementos de la fila
     * B (Bi,*) a los procesos en la columna j del mesh de procesos
     * pj,*)
     */
    MPI_Log(INFO, "<%d> PASO 1 (SCATTER) \n", myrank);
    MPI_Scatter(&(B[(nlocal*coords[ROW])]), N*nlocal, bloqueB, 
                &(B[(nlocal*coords[ROW])]), N*nlocal, bloqueB, 
                other_rank, comm_col);
    
    // Calculamos el Resultado: Rx = Ac x B
    for (i = 0; i < nlocal; i++) {
        for (j = 0; j < N; j++)
            px[i * N + j] += A[i * N + j] * B[i];
    }
    MPI_Log(INFO, "<%d> BARRIER (ANTES) \n", myrank);
    val=MPI_Barrier(comm_col);
    MPI_Log(INFO, "<%d> BARRIER (DESPUES) \n", myrank);
    
    /*
     * Paso 2: Enviamos el Resultado en la direccion "y".
     */
    coords[COL] = mycoords[COL];
    coords[ROW] = mycoords[COL];
    /*
     * Operacion: Gather de filas
     * All-to-One reduction de los elementos de la fila de
     * B (Bi,*) a los procesos en la columna j del mesh de procesos
     * pj,*
     */
    MPI_Cart_rank(comm_row, coords, &other_rank);
    MPI_Log(INFO, "@%d@ PASO 2 (REDUCE) \n", myrank);
    MPI_Reduce(px, C, N*N, MPI_ELEMENT_T, MPI_SUM, other_rank, comm_row);
    
    MPI_Comm_free(&comm_2d); /* Free up communicator */
    MPI_Comm_free(&comm_row); /* Free up communicator */
    MPI_Comm_free(&comm_col); /* Free up communicator */

    free(px);
}
