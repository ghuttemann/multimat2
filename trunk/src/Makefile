##
## Variables globales
##
LIBS = -lpthread -lm
FLAGS= -Wall

##
## Regla que le dice a Make como "llegar" de un .c a un .o
##
.c.o:
	gcc $(FLAGS) -c $< -o $@

##
## Variable que contiene la lista de modulos que componen nuestro "proyecto"
## Se puede usar \ para indicar que continua en la siguiente linea (pero
## no olvidar poner TAB en la sgte. 
##
modulos = utils.o matrix.o config.o main.o 

## 
## Como este es el primer target (all), se elige automaticamente cuando no 
## se pasa ningun target a Make. Es costumbre que exista este target.
##
all: matrix-mult 

##
## Estas dependencias no se suelen listar "a mano". Se usa una herramienta
## que se llama "makedep" (o mkdep), que genera automaticamente las
## dependencias de archivos de cabecera. (No se hacen a mano porque deben
## ser exhaustivas y transitivas: a->b->c...).
##
utils.o:  utils.c utils.h
matrix.o: matrix.c matrix.h
config.o: config.c config.h
main.o:   main.c config.h

##
## Con es target construimos el proyecto
##
matrix-mult: $(modulos)
	gcc $(FLAGS) -o $@ $(modulos) $(LIBS)

##
## El target clean suele existir para "remover" los archivos temporales
## y el archivo que se genera como producto del proyecto.
##
## El - delante quiere decir que si el comando falla, Make continua; no
## falla.  El @ delante quiere decir que no muestre (que no haga echo) de
## los comandos que esta ejecutando.
## 
clean:
	rm -f *.o
	rm -f matrix-mult
