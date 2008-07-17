#include <mpi.h>
#include "utils.h"
#include "matrix.h"

/******************************************************************************
 ************************** D E F I N I C I O N E S ***************************
 ******************************************************************************/

#define MPI_ELEMENT_T MPI_FLOAT

/******************************************************************************
 ********************* F U N C I O N  P R I N C I P A L ***********************
 ******************************************************************************/
int main(int argc, char *argv[]) {
    
    MPI_Init(&argc, &argv);
    
    MPI_Exit(EXIT_SUCCESS);
}

/******************************************************************************
 ***************************** F U N C I O N E S ******************************
 ******************************************************************************/
void CannonMatrixMatrixMultiply(int N, element_t *A, element_t *B, element_t *C, MPI_Comm comm) {
    int i;
    int nlocal;
    int commSize, dims[2], periods[2];
    int myRank, myRank_2d, myCoords[2];
    int upRank, downRank, leftRank, rightRank, coords[2];
    int shiftSource, shiftDest;
    MPI_Status status;
    MPI_Comm comm_2d;

    /* Obtenemos información del comunicador */
    MPI_Comm_size(comm, &commSize);
    MPI_Comm_rank(comm, &myRank);

    /* Configuramos la topología cartesiana */
    dims[0] = dims[1] = sqrt(commSize);

    /* Hacemos que la comunicación sea cíclica */
    periods[0] = periods[1] = 1;

    /* Se crea la topología cartesiana con reordenamiento de rank */
    MPI_Cart_create(comm, 2, dims, periods, 1, &comm_2d);

    /* Obtenemos el rank y las coordenadas respecto a la nueva topología */
    MPI_Comm_rank(comm_2d, &myRank_2d);
    MPI_Cart_coords(comm_2d, myRank_2d, 2, myCoords);

    /* Calculamos el rank de los procesos de arriba y de la izquierda */
    MPI_Cart_shift(comm_2d, 0, -1, &rightRank, &leftRank);
    MPI_Cart_shift(comm_2d, 1, -1, &downRank, &upRank);

    /* Determine the dimension of the local matrix block */
    nlocal = N / dims[0];

    /* Realizamos el alineamiento inicial de la matriz A */
    MPI_Cart_shift(comm_2d, 0, -myCoords[0], &shiftSource, &shiftDest);
    MPI_Sendrecv_replace(A, nlocal*nlocal, MPI_ELEMENT_T, shiftDest, 1, 
                        shiftSource, 1, comm_2d, &status);

    /* Realizamos el alineamiento inicial de la matriz B */
    MPI_Cart_shift(comm_2d, 1, -myCoords[1], &shiftSource, &shiftDest);
    MPI_Sendrecv_replace(B, nlocal*nlocal, MPI_ELEMENT_T, shiftDest, 1, 
                        shiftSource, 1, comm_2d, &status);

    
    /* Ciclo MULTIPLICACON-CORRIMIENTO */
    for (i = 0; i < dims[0]; i++) {
        matrix_mult(A, B, C, nlocal);

        /* 
         * Enviamos la matriz A desde la izquierda y 
         * la recibimos desde la derecha 
         */
        MPI_Sendrecv_replace(A, nlocal*nlocal, MPI_ELEMENT_T, leftRank, 1, 
                            rightRank, 1, comm_2d, &status);

        /* 
         * Enviamos la matriz B hacia arriba y la 
         * recibimos desde abajo 
         */
        MPI_Sendrecv_replace(B, nlocal*nlocal, MPI_ELEMENT_T, upRank, 1, 
                            downRank, 1, comm_2d, &status);
    }

    /* Restauramos la distribución original de la matriz A */
    MPI_Cart_shift(comm_2d, 0, +myCoords[0], &shiftSource, &shiftDest);
    MPI_Sendrecv_replace(A, nlocal*nlocal, MPI_ELEMENT_T, shiftDest, 1, 
                        shiftSource, 1, comm_2d, &status);

    /* Restauramos la distribución original de la matriz B */
    MPI_Cart_shift(comm_2d, 1, +myCoords[1], &shiftSource, &shiftDest);
    MPI_Sendrecv_replace(B, nlocal*nlocal, MPI_ELEMENT_T, shiftDest, 1, 
                        shiftSource, 1, comm_2d, &status);

    /* Liberamos el comunicador 2D */
    MPI_Comm_free(&comm_2d);
}
