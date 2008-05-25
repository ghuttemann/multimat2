#include "utils.h"

void *xmalloc(size_t size) {
    void *ptr;
    
    if ((ptr = malloc(size)) == NULL)
        LOG(FATAL, "%s: %s", __func__, "Memoria no disponible.");
    
    return ptr;
}

bool is_number(char *str) {
	int i, len = strlen(str);
	
	for (i=0; i < len; i++)
		if (!isdigit(str[i]))
			return false;
	
	return true;
}

bool es_cuadrado_perfecto(int numero) {
	if (numero < 0)
		return false;
	
	double raiz_decimal = sqrt(numero);
	double raiz_entera  = trunc(raiz_decimal);
	
	if (raiz_decimal == raiz_entera)
		return true;
	
	return false;
}
