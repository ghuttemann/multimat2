#include <mpi.h>
#include "utils.h"
#include "matrix.h"

/******************************************************************************
 ************************** D E F I N I C I O N E S ***************************
 ******************************************************************************/

#define MPI_ELEMENT_T MPI_FLOAT

void cannon_matrix_multiply(int, element_t *, element_t *, element_t *, MPI_Comm);
int get_matrix_size(int, char **);

/******************************************************************************
 ********************* F U N C I O N  P R I N C I P A L ***********************
 ******************************************************************************/
int main(int argc, char *argv[]) {
    int myRank   = -1; 	// Posicion dentro del comunicador
	int commSize = 0;	// Tamaño del comunicador
	int matSize  = 0; 	// Tamaño de la matriz
    int blkSize  = 0; 	// Tamaño del bloque
    
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
	 * Obtenemos el ranking del proceso.
	 */
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    
    // Obtener tamaño de la matriz
    if ((matSize = get_matrix_size(argc, argv)) < 0) {
        if (myRank != 0)
            MPI_Exit(1);
        else {
            MPI_Log(FATAL, "Dimension de matriz debe ser un numero "
                    "entero y positivo (se recibio %s)", argv[1]);
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
     * Calculamos el tamaño de bloque.
     */
    blkSize = matSize / commSizeSqrt;
    
    /*
     * Creamos las matrices A, B y C
     */
    element_t *matA = GET_MEM(element_t, blkSize * blkSize);
    element_t *matB = GET_MEM(element_t, blkSize * blkSize);
    element_t *matC = GET_MEM(element_t, blkSize * blkSize);
    
    /*
     * Inicializamos las matrices A, B y C
     */
    matrix_fill(matA, blkSize);
    matrix_fill(matB, blkSize);
    matrix_clear(matC, blkSize);
    
    MPI_Log(INFO, "Matrices A, B y C creadas");
    
    /*
     * Ejecutamos el algoritmo de Cannon.
     */
    cannon_matrix_multiply(matSize, matA, matB, matC, MPI_COMM_WORLD);
    
    // Terminación
    MPI_Exit(EXIT_SUCCESS);
}

/******************************************************************************
 ***************************** F U N C I O N E S ******************************
 ******************************************************************************/
void cannon_matrix_multiply(int N, element_t *A, element_t *B, element_t *C, 
                                MPI_Comm comm) {
    
    int i, nlocal, rc1=0, rc2=0;
    int commSize, dims[2], periods[2];
    int myRank, myRank_2d, myCoords[2];
    int upRank, downRank, leftRank, rightRank;
    int shiftSource, shiftDest;
    MPI_Status status;
    MPI_Comm comm_2d;

    /* Obtenemos información del comunicador */
    MPI_Comm_size(comm, &commSize);
    MPI_Comm_rank(comm, &myRank);

    /* Configuramos la topología cartesiana */
    dims[0] = dims[1] = (int) sqrt(commSize);

    /* Hacemos que la comunicación sea cíclica */
    periods[0] = periods[1] = 1;

    /* Se crea la topología cartesiana con reordenamiento de rank */
    MPI_Cart_create(comm, 2, dims, periods, 1, &comm_2d);

    /* Obtenemos el rank y las coordenadas respecto a la nueva topología */
    MPI_Comm_rank(comm_2d, &myRank_2d);
    MPI_Cart_coords(comm_2d, myRank_2d, 2, myCoords);

    /* Calculamos el rank de los procesos de la izquierda y de arriba */
    MPI_Cart_shift(comm_2d, 0, -1, &rightRank, &leftRank);
    MPI_Cart_shift(comm_2d, 1, -1, &downRank, &upRank);

    /* Determinamos la dimensión de las matrices locales */
    nlocal = N / dims[0];

    /* Realizamos el alineamiento inicial de la matriz A */
    rc1 = MPI_Cart_shift(comm_2d, 0, -myCoords[0], &shiftSource, &shiftDest);
    rc2 = MPI_Sendrecv_replace(A, nlocal*nlocal, MPI_ELEMENT_T, shiftDest, 1, 
                               shiftSource, 1, comm_2d, &status);
    
    MPI_Log(INFO, "Alineamiento inicial de matriz A en proceso [%d, %d] (%d %d)", 
            myCoords[0], myCoords[1], rc1, rc2);

    /* Realizamos el alineamiento inicial de la matriz B */
    rc1 = MPI_Cart_shift(comm_2d, 1, -myCoords[1], &shiftSource, &shiftDest);
    rc2 = MPI_Sendrecv_replace(B, nlocal*nlocal, MPI_ELEMENT_T, shiftDest, 1, 
                               shiftSource, 1, comm_2d, &status);
    
    MPI_Log(INFO, "Alineamiento inicial de matriz B en proceso [%d, %d] (%d %d)", 
            myCoords[0], myCoords[1], rc1, rc2);

    
    /* 
     * Ciclo MULTIPLICACON-CORRIMIENTO
     *
     * Debemos iterar una cantidad igual a la
     * raíz cuadrada del número de procesos, ya
     * que cada proceso necesita dicha cantidad
     * de filas de A y columnas de B.
     */
    for (i = 0; i < dims[0]; i++) {
        matrix_mult(A, B, C, nlocal);
        
        MPI_Log(INFO, "Multiplicacion C=A*B en proceso [%d, %d]", 
                myCoords[0], myCoords[1]);

        /* 
         * Enviamos la matriz A desde la izquierda y 
         * la recibimos desde la derecha 
         */
        rc1 = MPI_Sendrecv_replace(A, nlocal*nlocal, MPI_ELEMENT_T, leftRank, 1, 
                                   rightRank, 1, comm_2d, &status);
        
        MPI_Log(INFO, "Corrimiento de matriz A en proceso [%d, %d] (%d)", 
                myCoords[0], myCoords[1], rc1);

        /* 
         * Enviamos la matriz B hacia arriba y la 
         * recibimos desde abajo 
         */
        rc1 = MPI_Sendrecv_replace(B, nlocal*nlocal, MPI_ELEMENT_T, upRank, 1, 
                                   downRank, 1, comm_2d, &status);
        
        MPI_Log(INFO, "Corrimiento de matriz B en proceso [%d, %d] (%d)", 
                myCoords[0], myCoords[1], rc1);
    }

    /* Restauramos la distribución original de la matriz A */
    rc1 = MPI_Cart_shift(comm_2d, 0, +myCoords[0], &shiftSource, &shiftDest);
    rc2 = MPI_Sendrecv_replace(A, nlocal*nlocal, MPI_ELEMENT_T, shiftDest, 1, 
                               shiftSource, 1, comm_2d, &status);
    
    MPI_Log(INFO, "Restaurar distribucion inicial de matriz A en proceso [%d, %d] (%d %d)", 
            myCoords[0], myCoords[1], rc1, rc2);

    /* Restauramos la distribución original de la matriz B */
    rc1 = MPI_Cart_shift(comm_2d, 1, +myCoords[1], &shiftSource, &shiftDest);
    rc2 = MPI_Sendrecv_replace(B, nlocal*nlocal, MPI_ELEMENT_T, shiftDest, 1, 
                               shiftSource, 1, comm_2d, &status);
    
    MPI_Log(INFO, "Restaurar distribucion inicial de matriz B en proceso [%d, %d] (%d %d)", 
            myCoords[0], myCoords[1], rc1, rc2);

    /* Liberamos el comunicador 2D */
    MPI_Comm_free(&comm_2d);
}

/*
 * Obtiene el tamaño de la matriz desde la línea de argumento. Si el
 * parámetro es inválido retorna un número negativo.
 */
int get_matrix_size(int argc, char *argv[]) {
    
	if (argc >= 2 && is_positive_integer(argv[1]))
        return atoi(argv[1]);
    
    return -1;
}
