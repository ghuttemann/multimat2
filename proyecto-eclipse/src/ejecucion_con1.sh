#!/bin/sh

FILE=matrix-mult_tiempos

for N in 400 800 1200 1600 2000 2400 2800 3200; do
	for H in 1 2 4 8 9 16 32 36 64; do
	echo "Ejecutando Concurrente $N 1d $H hilos"
		for i in 1 2 3; do
			./matrix-mult -a $N $N -b $N $N -h $H -t 1 -ni
			
			if test $N -lt 1000 then
                if test $H -lt 10 then
                    mv $FILE.csv $FILE-con1-0$N-0$H-$i.csv
                else
                    mv $FILE.csv $FILE-con1-0$N-$H-$i.csv
                fi
			else
    			if test $H -lt 10 then
                    mv $FILE.csv $FILE-con1-$N-0$H-$i.csv
                else
                    mv $FILE.csv $FILE-con1-$N-$H-$i.csv
                fi
			fi
		done
	done
done
