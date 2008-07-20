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

void print_parallel_time(double initTime, double endTime) {
    printf("\n\n%s\n\n  Tiempo total: %f (%s)  \n\n%s\n",
        "###################################",
        endTime - initTime,
        MPI_WTIME_IS_GLOBAL ? "GLOBAL" : "LOCAL",
        "###################################");
}