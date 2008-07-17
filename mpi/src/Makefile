##
## Variables globales
##
LIBS  = -lm -lmpich
FLAGS = -Wall -g
CC    = gcc

##
## Directorios
##
LIB_DIR = -L"./lib"
INC_DIR = -I"./include"
OBJ_DIR = ./build
BIN_DIR = ./bin


##
## Variable que contiene la lista de modulos que componen nuestro "proyecto"
## Se puede usar \ para indicar que continua en la siguiente linea (pero
## no olvidar poner TAB en la sgte. 
##
modulos       = $(OBJ_DIR)/utils.o $(OBJ_DIR)/matrix.o
mod_aleatorio = $(OBJ_DIR)/mm_2d_aleatorio.o
mod_ciclico   = $(OBJ_DIR)/mm_2d_ciclico.o
mod_cannon    = $(OBJ_DIR)/mm_cannon.o

## 
## Como este es el primer target (all), se elige automaticamente cuando no 
## se pasa ningun target a Make. Es costumbre que exista este target.
##
all: $(BIN_DIR)/mm_2d_aleatorio $(BIN_DIR)/mm_2d_ciclico $(BIN_DIR)/mm_cannon

##
## Estas dependencias no se suelen listar "a mano". Se usa una herramienta
## que se llama "makedep" (o mkdep), que genera automaticamente las
## dependencias de archivos de cabecera. (No se hacen a mano porque deben
## ser exhaustivas y transitivas: a->b->c...).
##
$(OBJ_DIR)/utils.o: utils.c utils.h
	$(CC) $(FLAGS) -c $< -o $@ $(INC_DIR)

$(OBJ_DIR)/matrix.o: matrix.c matrix.h utils.h
	$(CC) $(FLAGS) -c $< -o $@ $(INC_DIR)

$(OBJ_DIR)/mm_2d_aleatorio.o: mm_2d_aleatorio.c matrix.h utils.h
	$(CC) $(FLAGS) -c $< -o $@ $(INC_DIR)

$(OBJ_DIR)/mm_2d_ciclico.o: mm_2d_ciclico.c matrix.h utils.h
	$(CC) $(FLAGS) -c $< -o $@ $(INC_DIR)

$(OBJ_DIR)/mm_cannon.o: mm_cannon.c matrix.h utils.h
	$(CC) $(FLAGS) -c $< -o $@ $(INC_DIR)

##
## Con este target construimos el proyecto
##
$(BIN_DIR)/mm_2d_aleatorio: $(modulos) $(mod_aleatorio)
	$(CC) $(FLAGS) -o $@ $(modulos) $(mod_aleatorio) $(LIB_DIR) $(LIBS)

$(BIN_DIR)/mm_2d_ciclico: $(modulos) $(mod_ciclico)
	$(CC) $(FLAGS) -o $@ $(modulos) $(mod_ciclico) $(LIB_DIR) $(LIBS)

$(BIN_DIR)/mm_cannon: $(modulos) $(mod_cannon)
	$(CC) $(FLAGS) -o $@ $(modulos) $(mod_cannon) $(LIB_DIR) $(LIBS)

##
## El target clean suele existir para "remover" los archivos temporales
## y el archivo que se genera como producto del proyecto.
## 
clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(BIN_DIR)/*.exe
