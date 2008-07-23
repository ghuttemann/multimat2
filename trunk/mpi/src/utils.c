#include "utils.h"

void *xmalloc(size_t size) {
    void *ptr;

    if ((ptr = malloc(size)) == NULL)
        LOG(FATAL, "%s(): %s", __func__, "Error reservando memoria.");

    return ptr;
}

bool is_positive_integer(char *str) {
	int i, len = strlen(str);

	for (i=0; i < len; i++)
		if (!isdigit(str[i]))
			return false;

	return true;
}

bool is_perfect_square(int num) {
	if (num < 0)
		return false;

	double decimal_root = sqrt(num);
	double integer_root = (double) ((int) decimal_root);

	if (decimal_root == integer_root)
		return true;

	return false;
}

bool is_perfect_cube(int num) {
    if (num < 0)
		return false;

	double decimal_root = cbrt(num);
	double integer_root = (double) ((int) decimal_root);

	if (decimal_root == integer_root)
		return true;

	return false;
}

void print_parallel_time(double initTime, double endTime, int rank) {
    char proc_name[MPI_MAX_PROCESSOR_NAME + 1];
    int length;
    
    // Obtenemos el nombre del procesador.
    MPI_Get_processor_name(proc_name, &length);
    
    printf("\n\n%s\n\n  Tiempo %s(%d): %f (%s)  \n\n%s\n",
        "#####################################################################",
        proc_name, rank, endTime - initTime,
        MPI_WTIME_IS_GLOBAL ? "GLOBAL" : "LOCAL",
        "#####################################################################");
}

FILE *open_file(char *path, char *modo) {
	FILE *file;
	
    if ((file = fopen(path, modo)) == NULL) {
        fprintf(stderr, "Error abriendo archivo '%s' en modo '%s'", path, modo);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
	
	return file;
}
