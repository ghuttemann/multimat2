C�digo fuente y binarios
========================

Cada c�digo fuente comienza con el nombre "mm_" (matrix multiplication) y luego tiene el nombre
del algoritmo correspondiente. En "include" y "lib" se encuentran los archivos de cabeceras y
librer�as, respectivamente, para la compilaci�n en Windows.

En "bin" se encuentran los binarios y en "build" los c�digos objeto.


Compilaci�n
===========

Para compilar en Windows debe hacer "make" dentro del directorio de los fuentes. Necesita
el comando "make" y el compilador GCC 3.4.4 (Cygwin). Si compila en Linux, probablemente 
ya posea el comando "make" y el compilador GCC.

Si desea evitar que se impriman mensajes de depuraci�n durante la ejecuci�n de los programas
debe compilar con "make DEBUG=no".

Antes de cada compilaci�n completa, es recomendado ejecutar primero "make clean" para que
los binarios y c�digos objeto anteriores sean eliminados.


Ejecuci�n
=========

La ejecuci�n fue probada en Windows solamente, con MPICH NT 1.2.5, el cual necesita estar
instalado para que la misma pueda realizarse. Adem�s, si no tiene instalado Cygwin, debe
copiar la librer�a "cygwin1.dll" en "C:\WINDOWS\system32".

Si desea ejecutar en Linux, necesita recompilar los binarios. La librer�a de "cygwin1.dll"
ya no es necesaria.

Para ejecutar los programas localmente en una m�quina:

    mpirun -np <#proc> -localonly bin/<programa>.exe <N>

Para ejecutar en varias m�quinas, lanzando una determinada cantidad de procesos en cada 
una de ellas y utilizando una prioridad alta para la ejecuci�n:

    mpirun -hosts <#hosts> <host1> <#proc1> <host2> <#proc2> ... <hostK> <#procK> -priority 4:5 <dircomun>/<programa>.exe <N>