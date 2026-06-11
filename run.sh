#!/bin/bash
reps=20
cores=6
percentage=1
for c in 1 2 4 6
do
for func in sinoidal michaelwicz rosenbrock camel 
do
    echo -e "times\n" > times/${c}p/$func.out
    for i in {1..25} 
    do
        mpirun -np $c ./release/$func config/$func.cfg $func.out
        echo $percentage%
        ((percentage++))
    done
done
done