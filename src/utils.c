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
