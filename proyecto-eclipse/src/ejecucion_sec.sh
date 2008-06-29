#!/bin/sh

FILE=matrix-mult_tiempos

for N in 400 800 1200 1600 2000 2400 2800 3200; do
	echo "Ejecutando Secuencial $N"
	for i in 1 2 3; do
		./matrix-mult -a $N $N -b $N $N -ni
		
        if test $N -lt 1000 then
            mv $FILE.csv $FILE-sec0$N-$i.csv
        else
            mv $FILE.csv $FILE-sec$N-$i.csv
        fi
	done
done
