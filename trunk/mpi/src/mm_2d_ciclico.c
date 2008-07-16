#include <mpi.h>
#include "utils.h"
#include "matrix.h"
#include "mm_2d_ciclico.h"

#define matSize 400

/*
 * Función principal
 */
int main(int argc, char *argv[]) {
	int i=0, j=0, k=0, rc=0;

    MPI_Status status;

	int myRank   = -1; 	// Posicion dentro del comunicador
	int commSize = 0;	// Tamaño del comunicador
	//int matSize  = 0; 	// Tamaño de la matriz
	int blkSize  = 0; 	// Tamaño del bloque
    int endTag   = 0;   // Numero de tag para finalizar


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
    if (!check_comm_size(commSize, MIN_COMM_SIZE)) {
        if (myRank != 0)
            MPI_Exit(1);
        else {
            MPI_Log(FATAL, "Tamaño mínimo del comunicador: %d", MIN_COMM_SIZE);
        }
    }

	/*/ Obtener tamaño de la matriz
    if ((matSize = get_matrix_size(argc, argv)) < 0) {
        if (myRank != 0)
            MPI_Exit(1);
        else {
            MPI_Log(FATAL, "%s %s (%s '%s').",
                    "Dimension de matriz debe ser un numero",
                    "entero, positivo y cuadrado perfecto",
                    "se recibio", argv[1]);
        }
    }*/
    
    /*
     * El tamaño de bloque se calcula en función
     * al tamaño de la matriz, de manera a no realizar
     * comunicación de más.
     */
    blkSize = (int) sqrt(matSize);
    
    /*
     * El tag de finalización debe ser igual a la cantidad
     * de tareas. Debido a que las tareas se enumeran desde
     * cero hasta "matSize", el tag será igual a éste.
     */
    endTag = matSize;
    

	/*
	 * Cada bloque de la matriz C representa una tarea. Como cada
	 * bloque tiene "sqrt(matSize)==blkSize" elementos, existen
	 * "blkSize*blkSize==matSize" tareas.
	 *
	 * Si la cantidad de tareas, T, es mayor a la cantidad de procesos, P,
	 * la distribución inicial debería enviar una tarea a cada proceso.
	 * Por el contrario, si T es menor que P, la distribución inicial se
	 * reparte entre los primeros T procesos y los restantes no reciben
	 * ninguna tarea, terminando inmediatamente.
	 */
	int maximo = matSize < (commSize - 1) ? matSize : (commSize - 1);

	// Tamaños de buffers.
	int buffSize   = (matSize * blkSize * 2);
	int resultSize = matSize;

	// Buffers de envío de mensajes y recepción de resultados.
	element_t mensaje[buffSize];
	element_t resultado[resultSize];

	/************************************************************************
	 * Ejecución de los procesos maestro y esclavos.
	 ************************************************************************/
	if (myRank == 0) {
		/*
		 * Proceso maestro
		 */

		// Matrices A, B y C
		element_t matA[matSize * matSize];
		element_t matB[matSize * matSize];
		element_t matC[matSize * matSize];

		// Rellenar matrices A y B
		matrix_fill(matA, matSize);
		matrix_fill(matB, matSize);
		MPI_Log(INFO, "Matrices A y B creadas");
        

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
		coord_2d tareas[matSize];
		k=0;
		for (i=0; i < matSize; i += blkSize) {
			for (j=0; j < matSize; j += blkSize) {
				tareas[k].primero = i;
				tareas[k].segundo = j;
				++k;
			}
		}
        
        /*
        fprintf(stderr, "Buffer    = %d\n", sizeof(mensaje)/sizeof(element_t));
        fprintf(stderr, "Resultado = %d\n", sizeof(resultado)/sizeof(element_t));
        fprintf(stderr, "Bloque    = %d\n", sizeof(blkSize));
        fprintf(stderr, "Matrices  = %d\n", sizeof(matC)/sizeof(element_t));
        fprintf(stderr, "Tareas    = %d\n", sizeof(tareas)/sizeof(coord_2d));
        */

		/*
		 * Realizar la comunicación de tareas
		 * entre los procesos esclavos.
		 *
		 * Inicialmente debemos mandar min(commSize-1, matSize)
		 * tareas, dependiendo de que hayan menos procesos
		 * que tareas o viceversa.
		 */

		// Comenzamos con el proceso 1.
		int destino = 1;

		/* 
         * Comienza la comunicación.
         * Iteramos sobre las tareas, enviando a los procesos.
         */
		for (k=0; k < matSize; k++) {
            
            // Construir mensaje.
			build_message(mensaje, matA, matB, matSize, blkSize,
						tareas[k].primero, tareas[k].segundo);
            
            MPI_Log(INFO, "Mensaje %d construido", k);

			if (destino <= maximo) {
				/*
				 * Enviar primer grupo de tareas.
				 */
				rc = MPI_Send(mensaje, buffSize, MPI_ELEMENT_T,
                            destino, k, MPI_COMM_WORLD);

				MPI_Log(INFO, "Mensaje %d enviado a proceso %d (%d)", k, destino, rc);

				++destino;
			}
			else {
				/*
				 * Si se llegó a "maximo", entonces debemos
				 * recibir las respuestas y guardar cada
				 * resultado en la correspondiente posición
				 * de la matriz C. Y seguir enviando tareas.
				 */

				// Recibir de cualquier proceso.
				rc = MPI_Recv(resultado, resultSize, MPI_ELEMENT_T,
                            MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

				MPI_Log(INFO, "Resultado %d recibido de proceso %d (%d)",
						status.MPI_TAG, status.MPI_SOURCE, rc);

				// Guardamos el resultado.
				save_result(matC, matSize, blkSize, resultado, 
                            tareas[status.MPI_TAG].primero, 
                            tareas[status.MPI_TAG].segundo);
                
				MPI_Log(INFO, "Resultado %d guardado", status.MPI_TAG);

				/*
				 * Enviamos otra tarea al proceso que acaba de responder.
				 */
                rc = MPI_Send(mensaje, buffSize, MPI_ELEMENT_T, status.MPI_SOURCE,
                            k, MPI_COMM_WORLD);

                MPI_Log(INFO, "Mensaje %d enviado a proceso %d (%d)", k, status.MPI_SOURCE, rc);
			}
		}
        
        MPI_Log(INFO, "Esperar ultimo(s) %d resultado(s)", maximo);

		/*
		 * Faltan recibir "maximo" resultados.
		 */
		for (k=0; k < maximo; k++) {
			// Recibir de cualquier proceso.
			rc = MPI_Recv(resultado, resultSize, MPI_ELEMENT_T,
                        MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			MPI_Log(INFO, "Resultado %d recibido de proceso %d (%d)",
					status.MPI_TAG, status.MPI_SOURCE, rc);

			// Guardamos el resultado.
			save_result(matC, matSize, blkSize, resultado,
                        tareas[status.MPI_TAG].primero, 
                        tareas[status.MPI_TAG].segundo);
            
			MPI_Log(INFO, "Resultado %d guardado", status.MPI_TAG);
		}
        
        MPI_Log(INFO, "Enviar mensajes de finalizacion a procesos");

		/*
		 * Debemos notificar a todos los esclavos que
		 * ha terminado el trabajo.
		 */
        for (k=1; k <= maximo; k++) {
			rc = MPI_Send(mensaje, buffSize, MPI_ELEMENT_T, k, endTag, MPI_COMM_WORLD);
            
            MPI_Log(INFO, "Mensaje de finalizacion para proceso %d (%d)", k, rc);
        }
	}
	else if (myRank <= maximo) {
		/*
		 * Procesos esclavos válidos
		 */

		// Recibir primer mensaje.
		rc = MPI_Recv(mensaje, buffSize, MPI_ELEMENT_T, 0, MPI_ANY_TAG,
                    MPI_COMM_WORLD, &status);
        
        MPI_Log(INFO, "Mensaje %d recibido por proceso %d (%d)", 
                status.MPI_TAG, myRank, rc);

		// Iterar hasta recibir el mensaje de terminación.
		while (status.MPI_TAG != endTag) {

			// Multiplicar bloque.
			multiply(mensaje, resultado, matSize, blkSize);
            
            MPI_Log(INFO, "Resultado %d calculado", status.MPI_TAG);

			// Enviar resultado a proceso maestro.
			rc = MPI_Send(resultado, resultSize, MPI_ELEMENT_T, 0,
                        status.MPI_TAG, MPI_COMM_WORLD);
            
            MPI_Log(INFO, "Resultado %d enviado a proceso %d (%d)", 
                    status.MPI_TAG, 0, rc);

			// Esperar otro bloque o mensaje de finalizacion.
			rc = MPI_Recv(mensaje, buffSize, MPI_ELEMENT_T, 0, MPI_ANY_TAG,
                        MPI_COMM_WORLD, &status);
            
            MPI_Log(INFO, "Mensaje %d recibido por proceso %d (%d)", 
                    status.MPI_TAG, myRank, rc);
		}
        
        MPI_Log(INFO, "Proceso %d finalizado", myRank);
	}
    
    // TAREAS DE FINALIZACION
    /*
    matrix_print(matA, matSize, stdout);
    printf("\n");
    matrix_print(matB, matSize, stdout);
    printf("\n");
    matrix_print(matC, matSize, stdout);
     */

	MPI_Exit(EXIT_SUCCESS);
}

void build_message(element_t *buffer, element_t *matA, element_t *matB,
					int n, int blkSize, int posFil, int posCol) {

	int i, j, k=0;

	// Copiar "b" filas de matA
	for (i=posFil; i < posFil + blkSize; i++)
	for (j=0; j < n; j++)
		buffer[k++] = matrix_val(matA, n, i, j);

	// Copiar "b" columnas de matB
	for (i=posCol; i < posCol + blkSize; i++)
	for (j=0; j < n; j++)
		buffer[k++] = matrix_val(matB, n, j, i);
}

int get_matrix_size(int argc, char *argv[]) {
    int tmp;
    
	if (argc == 2 && is_positive_integer(argv[1]) && 
        is_perfect_square(tmp = atoi(argv[1]))) {

		return tmp;
	}
    
    return -1;
}

bool check_comm_size(int realSize, int minSize) {
	if (realSize < minSize)
        return false;
    
    return true;
}

void save_result(element_t *matC, int n, int blkSize, element_t *resultado, 
                int posFil, int posCol) {

	/*
	 * Copiamos el bloque recibido al correspondiente
	 * bloque en la matriz C.
	 */
	int x, y, z=0;
	for (x=posFil; x < posFil + blkSize; x++)
	for (y=posCol; y < posCol + blkSize; y++)
		matrix_ref(matC, n, x, y) = resultado[z++];
}

void multiply(element_t *buffer, element_t *resultado, int n, int blkSize) {
	// Puntero temporal para bloque A
	element_t *blkA = buffer;

	// Puntero temporal para bloque B
	element_t *blkB = buffer + (n * blkSize);

	// Multiplicamos los bloques
	int i, j, k;    

	for (i=0; i < blkSize; i++)
	for (j=0; j < blkSize; j++)
	for (k=0; k < n; k++)
		matrix_ref(resultado, blkSize, i, j) +=
			matrix_val(blkA, n, i, k) * matrix_val(blkB, n, k, j);
}