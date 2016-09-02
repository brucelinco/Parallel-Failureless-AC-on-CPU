#!/bin/bash
gcc PFAC_CPU.c -o PFAC_CPU -O3
./PFAC_CPU patterns.txt inputs.txt
gcc PFAC_CPU_OMP.c -o PFAC_CPU_OMP -fopenmp -O3
./PFAC_CPU_OMP patterns.txt inputs.txt
gcc AC_CPU.c -o AC_CPU -O3 
./AC_CPU patterns.txt inputs.txt
gcc AC_CPU_OMP.c -o AC_CPU_OMP -fopenmp -O3 
./AC_CPU_OMP patterns.txt inputs.txt
