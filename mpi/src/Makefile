##
## Variables globales
##
LIBS  = -lm -lmpich
FLAGS = -Wall
CC    = gcc

##
## Directorios
##
LIB_DIR = -L"./lib"
INC_DIR = -I"./include"
OBJ_DIR = ./build
BIN_DIR = ./bin

##
## Si se pasa como argumento DEBUG=no, entonces
## no se realiza impresiones de depuración.
##
ifeq ($(DEBUG),no)
  DEF = -DNODEBUG
endif

##
## Variable que contiene la lista de modulos que componen nuestro "proyecto"
## Se puede usar \ para indicar que continua en la siguiente linea (pero
## no olvidar poner TAB en la sgte. 
##
modulos       = $(OBJ_DIR)/utils.o $(OBJ_DIR)/matrix.o
mod_aleatorio = $(OBJ_DIR)/mm_2d_aleatorio.o
mod_ciclico   = $(OBJ_DIR)/mm_2d_ciclico.o
mod_cannon    = $(OBJ_DIR)/mm_cannon.o
mod_dns       = $(OBJ_DIR)/mm_dns.o
mod_diagonal  = $(OBJ_DIR)/mm_2d_diagonal.o

## 
## Como este es el primer target (all), se elige automaticamente cuando no 
## se pasa ningun target a Make. Es costumbre que exista este target.
##
all: $(BIN_DIR)/mm_2d_aleatorio $(BIN_DIR)/mm_2d_ciclico \
	$(BIN_DIR)/mm_cannon $(BIN_DIR)/mm_dns $(BIN_DIR)/mm_2d_diagonal

##
## Estas dependencias no se suelen listar "a mano". Se usa una herramienta
## que se llama "makedep" (o mkdep), que genera automaticamente las
## dependencias de archivos de cabecera. (No se hacen a mano porque deben
## ser exhaustivas y transitivas: a->b->c...).
##
$(OBJ_DIR)/utils.o: utils.c utils.h
	$(CC) $(FLAGS) -c $< -o $@ $(DEF) $(INC_DIR)

$(OBJ_DIR)/matrix.o: matrix.c matrix.h utils.h
	$(CC) $(FLAGS) -c $< -o $@ $(DEF) $(INC_DIR)

$(OBJ_DIR)/mm_2d_aleatorio.o: mm_2d_aleatorio.c matrix.h utils.h
	$(CC) $(FLAGS) -c $< -o $@ $(DEF) $(INC_DIR)

$(OBJ_DIR)/mm_2d_ciclico.o: mm_2d_ciclico.c matrix.h utils.h
	$(CC) $(FLAGS) -c $< -o $@ $(DEF) $(INC_DIR)

$(OBJ_DIR)/mm_cannon.o: mm_cannon.c matrix.h utils.h
	$(CC) $(FLAGS) -c $< -o $@ $(DEF) $(INC_DIR)

$(OBJ_DIR)/mm_dns.o: mm_dns.c matrix.h utils.h
	$(CC) $(FLAGS) -c $< -o $@ $(DEF) $(INC_DIR)

$(OBJ_DIR)/mm_2d_diagonal.o: mm_2d_diagonal.c matrix.h utils.h
	$(CC) $(FLAGS) -c $< -o $@ $(DEF) $(INC_DIR)

##
## Con este target construimos el proyecto
##
$(BIN_DIR)/mm_2d_aleatorio: $(modulos) $(mod_aleatorio)
	$(CC) $(FLAGS) -o $@ $(modulos) $(mod_aleatorio) $(DEF) $(LIB_DIR) $(LIBS)

$(BIN_DIR)/mm_2d_ciclico: $(modulos) $(mod_ciclico)
	$(CC) $(FLAGS) -o $@ $(modulos) $(mod_ciclico) $(DEF) $(LIB_DIR) $(LIBS)

$(BIN_DIR)/mm_cannon: $(modulos) $(mod_cannon)
	$(CC) $(FLAGS) -o $@ $(modulos) $(mod_cannon) $(DEF) $(LIB_DIR) $(LIBS)

$(BIN_DIR)/mm_dns: $(modulos) $(mod_dns)
	$(CC) $(FLAGS) -o $@ $(modulos) $(mod_dns) $(DEF) $(LIB_DIR) $(LIBS)

$(BIN_DIR)/mm_2d_diagonal: $(modulos) $(mod_diagonal)
	$(CC) $(FLAGS) -o $@ $(modulos) $(mod_diagonal) $(DEF) $(LIB_DIR) $(LIBS)

##
## El target clean suele existir para "remover" los archivos temporales
## y el archivo que se genera como producto del proyecto.
## 
clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(BIN_DIR)/*.exe
