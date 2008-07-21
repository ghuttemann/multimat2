#include "utils.h"
#include "matrix.h"

/******************************************************************************
 ************************** D E F I N I C I O N E S ***************************
 ******************************************************************************/

#define MIN_COMM_SIZE 2
#define MPI_ELEMENT_T MPI_FLOAT

/*
 * Tipo de dato que representa
 * una coordenada en 2D.
 */
typedef struct {
	int primero, segundo;
} coord_2d;

void build_message(element_t *, element_t *, element_t *, int, int, int, int);
int get_matrix_size(int, char **);
void save_result(element_t *, int, int, element_t *, int, int);
void multiply(element_t *, element_t *, int, int);
void procesoMaestro(int, int, int, bool);
void procesoEsclavo(int, int, int, int);


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
	 * Obtenemos el ranking del proceso.
	 */
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	/*
	 * Verificamos que el tamaño del comunicador
	 * sea adecuado.
	 */
    if (commSize < MIN_COMM_SIZE) {
        if (myRank != 0)
            MPI_Exit(1);
        else {
            MPI_Log(FATAL, "Tamaño mínimo del comunicador: %d", MIN_COMM_SIZE);
        }
    }

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
     * Cantidad de tareas debe ser mayor o igual
     * a la cantidad de procesos esclavos.
     */
    if (matSize < commSize - 1)
        MPI_Log(FATAL, "%s (%d) %s (%d)", "La cantidad de tareas", matSize,
                "debe ser mayor o igual que la cantidad de procesos esclavos", 
                commSize - 1);
    
    /*
     * El tamaño de bloque se calcula en función
     * al tamaño de la matriz, de manera a no realizar
     * comunicación de más.
     */
    blkSize = (int) sqrt(matSize);
    
    /*
     * Establecemos la bandera que indica si se
     * imprimirán o no las matrices.
     */
    if (argc >= 3 && strcmp("p", argv[2]) == 0)
        printMatrix = true;
    else
        printMatrix = false;
    

	/************************************************************************
	 * Ejecución de los procesos maestro y esclavos.
	 ************************************************************************/
	if (myRank == 0)
        procesoMaestro(matSize, blkSize, commSize, printMatrix);
	else
        procesoEsclavo(matSize, blkSize, commSize, myRank);

    // Terminación
	MPI_Exit(EXIT_SUCCESS);
}

/******************************************************************************
 ***************************** F U N C I O N E S ******************************
 ******************************************************************************/

/*
 * Construye un mensaje correspondiente a una tarea. El mismo es enviado
 * por el proceso maestro a algún esclavo.
 *
 * El mensaje contiene "blkSize" filas de la matriz A y "blkSize" columnas 
 * de la matriz B.
 *
 * Los enteros "posFil" y "posCol" indican, respectivamente, desde qué fila
 * de la matriz A y desde qué columna de la matriz B debe comenzar a copiarse
 * en el buffer.
 */
void build_message(element_t *buffer, element_t *matA, element_t *matB,
					int n, int blkSize, int posFil, int posCol) {

	int i, j, k=0;

	// Copiar "b" filas de matA
	for (i=posFil; i < posFil + blkSize; i++)
	for (j=0; j < n; j++)
		buffer[k++] = matA[matrix_map(n, i, j)];

	// Copiar "b" columnas de matB
	for (i=posCol; i < posCol + blkSize; i++)
	for (j=0; j < n; j++)
        buffer[k++] = matB[matrix_map(n, j, i)];
}

/*
 * Obtiene el tamaño de la matriz desde la línea de argumento. Si el
 * parámetro es inválido retorna un número negativo.
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
 * Guarda los resultados calculados en la correspondiente ubicación 
 * de la matriz C.
 */
void save_result(element_t *matC, int n, int blkSize, element_t *resultado, 
                int posFil, int posCol) {

	/*
	 * Copiamos el bloque recibido al correspondiente
	 * bloque en la matriz C.
	 */
	int x, y, z=0;
	for (x=posFil; x < posFil + blkSize; x++)
	for (y=posCol; y < posCol + blkSize; y++)
		matC[matrix_map(n, x, y)] = resultado[z++];
}

/*
 * Calcula la multiplicación para un bloque
 * dado de la matriz C.
 */
void multiply(element_t *mensaje, element_t *resultado, int n, int blkSize) {
	// Puntero temporal para bloque A
	element_t *blkA = mensaje;

	// Puntero temporal para bloque B
	element_t *blkB = mensaje + (n * blkSize);

	// Multiplicamos los bloques
	int i, j, k;    

	for (i=0; i < blkSize; i++)
	for (j=0; j < blkSize; j++)
	for (k=0; k < n; k++)
		resultado[matrix_map(blkSize, i, j)] +=
			blkA[matrix_map(n, i, k)] * blkB[matrix_map(n, j, k)];
}

/*
 * Función que contiene el código para el proceso esclavo.
 */
void procesoMaestro(int matSize, int blkSize, int commSize, bool printMatrix) {
    int i=0, j=0, k=0, rc=0;

    MPI_Status status;
    
    // Tamaños de buffers.
	int mensSize   = (matSize * blkSize * 2);
	int resultSize = matSize;

	// Buffers de envío de mensajes y recepción de resultados.
	element_t *mensaje   = GET_MEM(element_t, mensSize);
	element_t *resultado = GET_MEM(element_t, resultSize);
    
    // Matrices A, B y C
    element_t *matA = GET_MEM(element_t, matSize * matSize);
    element_t *matB = GET_MEM(element_t, matSize * matSize);
    element_t *matC = GET_MEM(element_t, matSize * matSize);

    // Rellenar matrices A y B, cerar matriz C
    matrix_fill(matA, matSize * matSize);
    matrix_fill(matB, matSize * matSize);
    matrix_clear(matC, matSize * matSize);
    MPI_Log(INFO, "Matrices A, B y C creadas");

    // Inicio control tiempo.
    double initTime = MPI_Wtime();
    
    /*
     * Construir tareas.
     *
     * Se almacena el inicio de cada uno de los
     * bloques de la matriz C que constituyen
     * las tareas.
     *
     * El tamaño de cada bloque es "blkSize"
     *
     */
    coord_2d *tareas = GET_MEM(coord_2d, matSize);
    k=0;
    for (i=0; i < matSize; i += blkSize) {
        for (j=0; j < matSize; j += blkSize) {
            tareas[k].primero = i;
            tareas[k].segundo = j;
            ++k;
        }
    }

    /* 
     * Comienza la comunicación.
     * 
     * Iteramos sobre las tareas, enviando a 
     * los procesos cíclicamente.
     */
    int sent_tasks, proc;
    int avail_tasks = matSize;

    while (avail_tasks > 0) {
        sent_tasks = 0;

        /* 
         * Enviar cíclicamente a los procesos.
         */
        for (proc=1; proc < commSize; proc++) {

            /*
             * Puede que la cantidad de tareas llegue
             * a cero antes de enviar a todos los procesos
             * del ciclo actual (caso que la cantidad de
             * tareas no sea divisible exactamente entre
             * la cantidad de procesos), entonces debemos
             * verificar para no enviar de más.
             */
            if (avail_tasks > 0) {

                // Construir mensaje.
                build_message(mensaje, matA, matB, matSize, blkSize,
                              tareas[matSize - avail_tasks].primero,
                              tareas[matSize - avail_tasks].segundo);

                MPI_Log(INFO, "Mensaje %d construido", matSize - avail_tasks);

                // Enviar mensaje.
                rc = MPI_Send(mensaje, mensSize, MPI_ELEMENT_T,
                              proc, matSize - avail_tasks, MPI_COMM_WORLD);

                MPI_Log(INFO, "Mensaje %d enviado a proceso %d (%d)",
                        matSize - avail_tasks, proc, rc);

                // Actualizar contadores
                ++sent_tasks;
                --avail_tasks;
            }
            else {
                /*
                 * Ya no hay tareas, no necesitamos
                 * seguir iterando sobre los procesos.
                 */
                break;
            }
        }

        /*
         * Ahora debemos recibir una cantidad de resultados
         * igual a la cantidad de mensajes enviados.
         */
        for (proc=1; proc <= sent_tasks; proc++) {

            // Recibimos el resultado.
            rc = MPI_Recv(resultado, resultSize, MPI_ELEMENT_T,
                          proc, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            MPI_Log(INFO, "Resultado %d recibido de proceso %d (%d)",
                    status.MPI_TAG, status.MPI_SOURCE, rc);

            // Guardamos el resultado.
            save_result(matC, matSize, blkSize, resultado,
                        tareas[status.MPI_TAG].primero,
                        tareas[status.MPI_TAG].segundo);

            MPI_Log(INFO, "Resultado %d guardado", status.MPI_TAG);
        }
    }

    // Fin control tiempo.
    double endTime = MPI_Wtime();

    /*
     * Impresión de las matrices.
     */
    if (printMatrix) {
        matrix_print(matA, matSize, matSize, stdout);
        printf("\n");
        matrix_print(matB, matSize, matSize, stdout);
        printf("\n");
        matrix_print(matC, matSize, matSize, stdout);
    }

    // Liberar buffers
    free(matA);
    free(matB);
    free(matC);
    free(tareas);
    free(mensaje);
    free(resultado);

    print_parallel_time(initTime, endTime);
}

/*
 * Función que contiene el código para los procesos esclavos.
 */
void procesoEsclavo(int matSize, int blkSize, int commSize, int myRank) {
    int k=0, rc=0;

    MPI_Status status;
    
    // Tamaños de buffers.
	int mensSize   = (matSize * blkSize * 2);
	int resultSize = matSize;

	// Buffers de envío de mensajes y recepción de resultados.
	element_t *mensaje   = GET_MEM(element_t, mensSize);
	element_t *resultado = GET_MEM(element_t, resultSize);
    
    // Cantidad de tareas para el proceso.
    int taskCount = matSize / (commSize - 1);
    
    /*
     * Si el resto de la división de tareas entre procesos 
     * esclavos es mayor a cero, los primeros procesos deberán 
     * recibir una tarea más que los demás.
     */
    if (myRank <= (matSize % (commSize - 1))
        ++taskCount;
    
    /*
     * Recibir taskCount tareas
     */
    for (k=0; k < taskCount; k++) {
        
        // Recibir tarea.
        rc = MPI_Recv(mensaje, mensSize, MPI_ELEMENT_T, 0, MPI_ANY_TAG,
                      MPI_COMM_WORLD, &status);

        MPI_Log(INFO, "Mensaje %d recibido por proceso %d (%d)",
                status.MPI_TAG, myRank, rc);
        
        // Cerar matriz resultado
        matrix_clear(resultado, blkSize * blkSize);

        // Multiplicar bloque.
        multiply(mensaje, resultado, matSize, blkSize);

        MPI_Log(INFO, "Resultado %d calculado", status.MPI_TAG);

        // Enviar resultado a proceso maestro.
        rc = MPI_Send(resultado, resultSize, MPI_ELEMENT_T, 0,
                      status.MPI_TAG, MPI_COMM_WORLD);

        MPI_Log(INFO, "Resultado %d enviado a proceso %d (%d)",
                status.MPI_TAG, 0, rc);
    }
    
    MPI_Log(INFO, "Proceso %d finalizado", myRank);

    free(mensaje);
    free(resultado);
}
