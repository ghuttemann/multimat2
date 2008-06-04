#!/bin/sh

FILE=matrix-mult_tiempos

for N in 200 400 800 1600; do
	echo "Ejecutando Secuencial $N"
	for i in 1 2 3; do
		./matrix-mult -a $N $N -b $N $N -ni
		mv $FILE.csv $FILE-sec0$N-$i.csv
	done
done
