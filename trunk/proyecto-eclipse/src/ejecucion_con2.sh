#!/bin/sh

FILE=matrix-mult_tiempos

for N in 200 400 800 1600; do
	for H in 4 9 16; do
	echo "Ejecutando Concurrente $N 2d $H hilos"
		for i in 1 2 3; do
			./matrix-mult -a $N $N -b $N $N -h $H -t 2 -ni
			mv $FILE.csv $FILE-con1-0$N-0$H-$i.csv
		done
	done
done
