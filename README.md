# Parallel Failureless Aho-Corasick algorithm on CPUs
## Created by Cheng-Hung Lin 2016/9/1
## CPU implementations:
### AC_CPU.c is the implementation of traditional single-threaded Aho-Corasick algorithm on CPU.
### AC_CPU_OMP.c is the implementation of traditional multi-threaded Aho-Corasick algorithm on CPU.
### PFAC_CPU.c is the implementation of single-threaded Parallel Failureless Aho-Corasick algorithm on CPU.
### PFAC_CPU_OMP.c is the implementation of multi-threaded Parallel Failureless Aho-Corasick algorithm on CPU.

###Compile:
###gcc AC_CPU.c -o AC_CPU -O3 
###gcc AC_CPU_OMP.c -o AC_CPU_OMP -fopenmp -O3 
###gcc PFAC_CPU.c -o PFAC_CPU -O3
###gcc PFAC_CPU_OMP.c -o PFAC_CPU_OMP -fopenmp -O3

##Run:
###1. prepare pattern file and input file. The pattern file can contain multiple lines of patterns.
###2. For example, the patterns.txt contains five patterns as follows. 
####she 
####he
####his
####the
####her 
###3. Run the following commands or the script "compileAndTest.sh"
###./AC_CPU patterns.txt inputs.txt
###./AC_CPU_OMP patterns.txt inputs.txt
###./PFAC_CPU patterns.txt inputs.txt
###./PFAC_CPU_OMP patterns.txt inputs.txt


 