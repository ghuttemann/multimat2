#ifndef _MM_2D_CICLICO_H
#define	_MM_2D_CICLICO_H

#define MIN_COMM_SIZE 2
#define MPI_ELEMENT_T MPI_FLOAT

/*
 * Tipo de dato que representa
 * una coordenada en 2D.
 */
typedef struct {
	int primero, segundo;
} coord_2d;

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
					int n, int blkSize, int posFil, int posCol);

/*
 * Obtiene el tamaño de la matriz desde la línea de argumento. Si el
 * parámetro es inválido retorna un número negativo.
 */
int get_matrix_size(int argc, char *argv[]);

/*
 * Verifica que el tamaño del comunicador no sea menor al mínimo permitido. 
 */
bool check_comm_size(int realSize, int minSize);

/*
 * Guarda los resultados calculados en la correspondiente ubicación 
 * de la matriz C.
 */
void save_result(element_t *matC, int n, int blkSize, element_t *resultado, 
                int posFil, int posCol);

/*
 * Calcula la multiplicación para un bloque
 * dado de la matriz C.
 */
void multiply(element_t *buffer, element_t *resultado, int n, int blkSize);


#endif	/* _MM_2D_CICLICO_H */

