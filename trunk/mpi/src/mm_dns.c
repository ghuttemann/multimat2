#include "utils.h"
#include "matrix.h"

/******************************************************************************
 ************************** D E F I N I C I O N E S ***************************
 ******************************************************************************/

#define MPI_ELEMENT_T MPI_FLOAT

#define DIM_I 0     // Posición de la dimensión i
#define DIM_J 1     // Posición de la dimensión j
#define DIM_K 2     // Posición de la dimensión k
#define NDIMS 3     // Cantidad de dimensiones

int get_matrix_size(int, char **);
void dns_matrix_multiply(int, element_t *, element_t *, 
                        element_t *, element_t *, MPI_Comm);

/******************************************************************************
 ********************* F U N C I O N  P R I N C I P A L ***********************
 ******************************************************************************/
int main(int argc, char *argv[]) {
    int myRank   = -1; 	// Posicion dentro del comunicador
	int commSize = 0;	// Tamaño del comunicador
	int matSize  = 0; 	// Tamaño de la matriz
    int blkSize  = 0; 	// Tamaño del bloque
    bool printMatrix;   // Bandera para impresión de matrices
    
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
            MPI_Log(FATAL, "Dimension de matriz debe ser un numero "
                    "entero y positivo (se recibio %s)", argv[1]);
        }
    }
    
    /*
	 * Verificamos que el tamaño del comunicador
	 * sea adecuado.
	 */
    if (!is_perfect_cube(commSize)) {
        if (myRank != 0)
            MPI_Exit(1);
        else {
            MPI_Log(FATAL, "Tamaño del comunicador (%d) "
                    "debe ser cubo perfecto", commSize);
        }
    }
    
    /*
     * Además, la dimensión de las matrices debe
     * ser divisible entre la raíz cúbica del
     * tamaño del comunicador (cantidad de procesos).
     */
    int commSizeCbrt = (int) cbrt(commSize);
    
    if (matSize % commSizeCbrt != 0) {
        if (myRank != 0)
            MPI_Exit(1);
        else {
            MPI_Log(FATAL, "Dimension de matriz (%d) debe ser divisible "
                    "entre raiz cúbica del tamaño del comunicador (%d)", 
                    matSize, 
                    commSizeCbrt);
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
    
    /**************************************************************************
     * Debemos crear la topología de grilla 3D
     * para comenzar a trabajar.
     **************************************************************************/
    MPI_Comm comm_3d;       // Comunicador en grilla 3D
    int myRank_3d;          // Rank en la grilla 3D
    int myCoords[NDIMS];    // Coordenadas en la grilla 3D
    int dims[NDIMS], periods[NDIMS];
    
    // Configuramos la topología cartesiana
    dims[DIM_I] = dims[DIM_J] = dims[DIM_K] = (int) cbrt(commSize);
    
    MPI_Debug(INFO, "Procesos en la grilla (%d, %d, %d)", 
            dims[DIM_I], dims[DIM_J], dims[DIM_K]);
    
    // No necesitamos que la comunicación sea cíclica
    periods[DIM_I] = periods[DIM_J] = periods[DIM_K] = false;
    
    // Se crea la topología cartesiana
    MPI_Cart_create(MPI_COMM_WORLD, NDIMS, dims, periods, 0, &comm_3d);
    
    // Obtenemos el rank y las coordenadas respecto a la nueva topología
    MPI_Comm_rank(comm_3d, &myRank_3d);
    MPI_Cart_coords(comm_3d, myRank_3d, NDIMS, myCoords);
    
    /**************************************************************************/
    
    /*
     * Calculamos el tamaño de bloque.
     */
    blkSize = matSize / commSizeCbrt;
    
    
    /*
     * Creamos las matrices A, B, C
     */
    element_t *matA = GET_MEM(element_t, blkSize * blkSize);
    element_t *matB = GET_MEM(element_t, blkSize * blkSize);
    element_t *matC = GET_MEM(element_t, blkSize * blkSize);
    
    /*
     * Además necesitamos otra matriz para guardar
     * el resultado de las operaciones all-to-one reduction.
     */
    element_t *matResult = GET_MEM(element_t, blkSize * blkSize);
    
    /*
     * Los procesos del plano 0, deben inicializar las 
     * matrices A y B.
     */
    if (myCoords[DIM_K] == 0) {
        matrix_load(matA, blkSize, A);
        matrix_load(matB, blkSize, B);
    }
    
    if (printMatrix && myCoords[DIM_K] == 0) {
        char filenameA[100];
        char filenameB[100];
        
        sprintf(filenameA, "matrixA-%d-%d-%d.txt", 
                myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_K]);
        sprintf(filenameB, "matrixB-%d-%d-%d.txt", 
                myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_K]);
        
        FILE *fileA = open_file(filenameA, "w");
        FILE *fileB = open_file(filenameB, "w");
        
        matrix_print(matA, blkSize, blkSize, fileA);
        matrix_print(matB, blkSize, blkSize, fileB);
        
        fclose(fileA);
        fclose(fileB);
    }
    
    // Inicializamos las matrices C y Result
    matrix_load(matC, blkSize, C);
    matrix_load(matResult, blkSize, C);
    
    // Inicio control tiempo.
    double initTime = MPI_Wtime();
    
    // Ejecutamos el algoritmo de Cannon.
    dns_matrix_multiply(blkSize, matA, matB, matC, matResult, comm_3d);
    
    // Fin control tiempo.
    double endTime = MPI_Wtime();
    
    /*
     * Impresión de las matrices (solo el plano cero).
     */
    if (printMatrix && myCoords[DIM_K] == 0) {
        char filenameC[100];
        
        sprintf(filenameC, "matrizC-%d-%d-%d.txt", 
                myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_K]);
        
        FILE *fileC = open_file(filenameC, "w");
        
        matrix_print(matResult, blkSize, blkSize, fileC);
        
        fclose(fileC);
    }
    
    // Liberar buffers
    free(matA);
    free(matB);
    free(matC);
    free(matResult);
    
    print_parallel_time(initTime, endTime, myRank);
    
    // Liberamos el comunicador 3D
    MPI_Comm_free(&comm_3d);
    
    // Terminación
    MPI_Exit(EXIT_SUCCESS);
}

/******************************************************************************
 ***************************** F U N C I O N E S ******************************
 ******************************************************************************/
void dns_matrix_multiply(int nlocal, element_t *A, element_t *B, element_t *C, 
                            element_t *R, MPI_Comm comm_3d) {
    
    int myRank_3d;
    int myCoords[NDIMS];
    MPI_Status status;
    
    /* 
     * Cada proceso obtiene el rank y las coordenadas 
     * respecto a la topología 3D.
     */
    MPI_Comm_rank(comm_3d, &myRank_3d);
    MPI_Cart_coords(comm_3d, myRank_3d, NDIMS, myCoords);
    
    
    /*************************************************************************
     **************************** F A S E   1 ********************************
     *************************************************************************/
    
    /* 1.1
     * 
     * Los procesos Pij0 (plano 0) deben transmitir
     * las columnas de A a los procesos Pijj (j > 0).
     */
    if (myCoords[DIM_K] == 0 && myCoords[DIM_J] > 0) {
        // Obtener rank del destino
        int destRank;
        int destCoords[NDIMS] = {myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_J]};
        MPI_Cart_rank(comm_3d, destCoords, &destRank);
        
        // Enviar A
        MPI_Send(A, nlocal * nlocal, MPI_ELEMENT_T, destRank, 1, comm_3d);
        
        MPI_Debug(INFO, "P[%d, %d, %d] envia A a P[%d, %d, %d]", 
                myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_K],
                destCoords[DIM_I], destCoords[DIM_J], destCoords[DIM_K]);
    }
    
    /* 1.2
     * 
     * Los procesos Pijk (k > 0 && j=k) deben recibir A de Pij0.
     */
    if (myCoords[DIM_K] > 0 && myCoords[DIM_J] == myCoords[DIM_K]) {
        // Obtener rank del origen
        int sourceRank;
        int sourceCoords[NDIMS] = {myCoords[DIM_I], myCoords[DIM_J], 0};
        MPI_Cart_rank(comm_3d, sourceCoords, &sourceRank);
        
        // Recibir A
        MPI_Recv(A, nlocal * nlocal, MPI_ELEMENT_T, sourceRank, 1, comm_3d, &status);
        
        MPI_Debug(INFO, "P[%d, %d, %d] recibe A de P[%d, %d, %d]", 
                myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_K],
                sourceCoords[DIM_I], sourceCoords[DIM_J], sourceCoords[DIM_K]);
    }
    
    /* 1.3
     * 
     * Los procesos Pij0 (plano 0) deben transmitir
     * las filas de B a los procesos Piji (i > 0).
     */
    if (myCoords[DIM_K] == 0 && myCoords[DIM_I] > 0) {
        // Obtener rank del destino
        int destRank;
        int destCoords[NDIMS] = {myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_I]};
        MPI_Cart_rank(comm_3d, destCoords, &destRank);
        
        // Enviar B
        MPI_Send(B, nlocal * nlocal, MPI_ELEMENT_T, destRank, 1, comm_3d);
        
        MPI_Debug(INFO, "P[%d, %d, %d] envia B a P[%d, %d, %d]", 
                myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_K],
                destCoords[DIM_I], destCoords[DIM_J], destCoords[DIM_K]);
    }
    
    /* 1.4
     * 
     * Los procesos Pijk (k > 0 && i=k) deben recibir B de Pij0.
     */
    if (myCoords[DIM_K] > 0 && myCoords[DIM_I] == myCoords[DIM_K]) {
        // Obtener rank del origen
        int sourceRank;
        int sourceCoords[NDIMS] = {myCoords[DIM_I], myCoords[DIM_J], 0};
        MPI_Cart_rank(comm_3d, sourceCoords, &sourceRank);
        
        // Recibir B
        MPI_Recv(B, nlocal * nlocal, MPI_ELEMENT_T, sourceRank, 1, comm_3d, &status);
        
        MPI_Debug(INFO, "P[%d, %d, %d] recibe B de P[%d, %d, %d]", 
                myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_K],
                sourceCoords[DIM_I], sourceCoords[DIM_J], sourceCoords[DIM_K]);
    }
    
    /*************************************************************************
     **************************** F A S E   2 ********************************
     *************************************************************************/
    
    /* 2.1
     * 
     * Cada proceso que posee columnas de A debe transmitirlas
     * entre los procesos a lo largo del eje J, con una operación
     * one-to-all broadcast.
     */
    
    
    /* 2.1.1
     *
     * Para ello necesita crear un comunicador entre 
     * los procesos de la misma columna en cada plano.
     */
    int remain_dims_cols[NDIMS] = {false, true, false};
    MPI_Comm comm_cols;
    MPI_Cart_sub(comm_3d, remain_dims_cols, &comm_cols);
    
    MPI_Debug(INFO, "Subgrilla de Columnas creada por P[%d, %d, %d]",
            myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_K]);
    
    
    /* 2.1.2
     *
     * Los root de cada Bcast deben ser los procesos dentro
     * de los nuevos comunicadores cuyo rank es igual a la
     * coordenada k del proceso que ejecuta el Bcast.
     */
    int rootColsRank = myCoords[DIM_K];
    
    
    /* 2.1.3
     *
     * La operación Bcast es realizada por todos los 
     * procesos dentro del comunicador de columnas
     * de cada plano.
     */
    MPI_Bcast(A, nlocal * nlocal, MPI_ELEMENT_T, rootColsRank, comm_cols);
    
    MPI_Debug(INFO, "Bcast(%d) de A en columnas ejecutado por P[%d, %d, %d]", 
            rootColsRank, myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_K]);
    
    
    /* 2.2
     * 
     * Cada proceso que posee filas de B debe transmitirlas
     * entre los procesos a lo largo del eje I, con una operación
     * one-to-all broadcast.
     */
    
    
    /* 2.2.1
     *
     * Para ello necesita crear un comunicador entre 
     * los procesos de la misma fila en cada plano.
     */
    int remain_dims_rows[NDIMS] = {true, false, false};
    MPI_Comm comm_rows;
    MPI_Cart_sub(comm_3d, remain_dims_rows, &comm_rows);
    
    MPI_Debug(INFO, "Subgrilla de Filas creada por P[%d, %d, %d]",
            myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_K]);
    
    
    /* 2.2.2
     *
     * Los root de cada Bcast deben ser los procesos dentro
     * de los nuevos comunicadores cuyo rank es igual a la
     * coordenada k del proceso que ejecuta el Bcast.
     */
    int rootRowsRank = myCoords[DIM_K];
    
    
    /* 2.2.3
     *
     * La operación Bcast es realizada por todos los 
     * procesos dentro del comunicador de filas
     * de cada plano.
     */
    MPI_Bcast(B, nlocal * nlocal, MPI_ELEMENT_T, rootRowsRank, comm_rows);
    
    MPI_Debug(INFO, "Bcast(%d) de B en filas ejecutado por P[%d, %d, %d]", 
            rootRowsRank, myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_K]);
    
    /*
     * 2.3
     *
     * Multiplicar las submatrices A y B.
     */
    MPI_Debug(INFO, "Inicio de multiplicacion por P[%d, %d, %d]",
            myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_K]);
    
    matrix_mult(A, B, C, nlocal);
    
    MPI_Debug(INFO, "Fin de multiplicacion por P[%d, %d, %d]",
            myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_K]);
    
    
    /*************************************************************************
     **************************** F A S E   3 ********************************
     *************************************************************************/
    
    /*
     * 3
     *
     * Cada proceso de los planos k > 0 debe realizar una operación
     * all-to-one reduction hacia el proceso correspondiente en el
     * plano k=0.
     */
    
    
    /* 3.1
     * Debemos construir comunicadores tal que incluyan a los procesos
     * Pijk, tal que k=0,1,2,3,...,n-1.
     */
    int remain_dims_reduction[NDIMS] = {false, false, true};
    MPI_Comm comm_reduction;
    MPI_Cart_sub(comm_3d, remain_dims_reduction, &comm_reduction);
    
    MPI_Debug(INFO, "Subgrilla para Reduction creada por P[%d, %d, %d]",
            myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_K]);
    
    
    /* 3.2
     *
     * Los root de cada Reduce deben ser los procesos k=0
     * de los comunicadores para el reduction.
     */
    int rootReductionRank = 0;
    
    
    /* 3.3
     *
     * La operación Reduce es realizada por todos los 
     * procesos dentro del comunicador para reduction.
     */
    MPI_Reduce(C, R, nlocal * nlocal, MPI_ELEMENT_T, MPI_SUM, rootReductionRank, comm_reduction);
    
    MPI_Debug(INFO, "Reduce(%d) realizado por P[%d, %d, %d]", 
            rootReductionRank, myCoords[DIM_I], myCoords[DIM_J], myCoords[DIM_K]);
    
    // Liberar comunicadores
    MPI_Comm_free(&comm_cols);
    MPI_Comm_free(&comm_rows);
    MPI_Comm_free(&comm_reduction);
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
