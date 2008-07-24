Código fuente y binarios
========================

Cada código fuente comienza con el nombre "mm_" (matrix multiplication) y luego tiene el nombre
del algoritmo correspondiente. En "include" y "lib" se encuentran los archivos de cabeceras y
librerías, respectivamente, para la compilación en Windows.

En "bin" se encuentran los binarios y en "build" los códigos objeto.


Compilación
===========

Para compilar en Windows debe hacer "make" dentro del directorio de los fuentes. Necesita
el comando "make" y el compilador GCC 3.4.4 (Cygwin). Si compila en Linux, probablemente 
ya posea el comando "make" y el compilador GCC.

Si desea evitar que se impriman mensajes de depuración durante la ejecución de los programas
debe compilar con "make DEBUG=no".

Antes de cada compilación completa, es recomendado ejecutar primero "make clean" para que
los binarios y códigos objeto anteriores sean eliminados.


Ejecución
=========

La ejecución fue probada en Windows solamente, con MPICH NT 1.2.5, el cual necesita estar
instalado para que la misma pueda realizarse. Además, si no tiene instalado Cygwin, debe
copiar la librería "cygwin1.dll" en "C:\WINDOWS\system32".

Si desea ejecutar en Linux, necesita recompilar los binarios. La librería de "cygwin1.dll"
ya no es necesaria.

Para ejecutar los programas localmente en una máquina:

    mpirun -np <#proc> -localonly bin/<programa>.exe <N>

Para ejecutar en varias máquinas, lanzando una determinada cantidad de procesos en cada 
una de ellas y utilizando una prioridad alta para la ejecución:

    mpirun -hosts <#hosts> <host1> <#proc1> <host2> <#proc2> ... <hostK> <#procK> -priority 4:5 <dircomun>/<programa>.exe <N>