#!/usr/bin/bash

for numprocs in 1 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32
do
echo +++++++++ procs num= $numprocs >> procs-a
for try in 1 2 3 4 5
do
./montice "pow(x*x+y*y+z*z,1.5)" -3 3 "-1*pow(9-x*x,0.5)" "pow(9-x*x,0.5)" "-1*pow(9-x*x-y*y,0.5)" "pow(9-x*x-y*y,0.5)" 1000000000 $numprocs host_cluster >> procs-a
done
done

for numprocs in 1 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32
do
echo +++++++++ procs num= $numprocs >> procs-b
for try in 1 2 3 4 5
do
./montice "y/(y*y+z*z)" 1 2 3 x 0 "y*pow(3,0.5)" 1000000000 $numprocs host_cluster >> procs-b
done
done

for numprocs in 1 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32
do
echo +++++++++ procs num= $numprocs >> procs-c
for try in 1 2 3 4 5
do
./montice "pow(x*x+y*y,0.5)" 0 3 0 "pow(9-x*x,0.5)" 0 2 1000000000 $numprocs host_cluster >> procs-c
done
done

for numprocs in 1 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32
do
echo +++++++++ procs num= "$numprocs" >> procs-d
for try in 1 2 3 4 5
do
./montice "z*pow(4-x*x-y*y,0.5)" 0 2 0 "pow(4-x*x,0.5)" 0 "pow(4-x*x-y*y,0.5)" 1000000000 $numprocs host_cluster >> procs-d
done
done




