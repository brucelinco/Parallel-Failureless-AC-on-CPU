# Parallel Failureless Aho-Corasick algorithm on CPUs
## Created by Cheng-Hung Lin 2016/9/1
## CPU implementations:
### AC_CPU.c is the implementation of traditional single-threaded Aho-Corasick algorithm on CPU.
### AC_CPU_OMP.c is the implementation of traditional multi-threaded Aho-Corasick algorithm on CPU.
### PFAC_CPU.c is the implementation of single-threaded Parallel Failureless Aho-Corasick algorithm on CPU.
### PFAC_CPU_OMP.c is the implementation of multi-threaded Parallel Failureless Aho-Corasick algorithm on CPU.

### Compile:
`gcc AC_CPU.c -o AC_CPU -O3`

`gcc AC_CPU_OMP.c -o AC_CPU_OMP -fopenmp -O3`

`gcc PFAC_CPU.c -o PFAC_CPU -O3`

`gcc PFAC_CPU_OMP.c -o PFAC_CPU_OMP -fopenmp -O3`

### Run:
### 1. prepare pattern file and input file. The pattern file can contain multiple lines of patterns. For example, the patterns.txt contains five patterns as follows. 
#### she 
#### he
#### his
#### the
#### her 
### The inputs.txt contains a string "shethehishetheehershehehishe".
### 3. Run the following commands or the script "compileAndTest.sh"
`./AC_CPU patterns.txt inputs.txt`

`./AC_CPU_OMP patterns.txt inputs.txt`

`./PFAC_CPU patterns.txt inputs.txt`

`./PFAC_CPU_OMP patterns.txt inputs.txt`
### Match results:
### The match result file show the position and matched patterns as follows.
### For example:
#### At position    0, match pattern 1
#### At position    1, match pattern 2
#### At position    3, match pattern 4
#### At position    4, match pattern 2
#### At position    6, match pattern 3
#### At position    8, match pattern 1
#### At position    9, match pattern 2
#### At position   11, match pattern 4
#### At position   12, match pattern 2
#### At position   15, match pattern 5
#### At position   18, match pattern 1
#### At position   19, match pattern 2
#### At position   21, match pattern 2
#### At position   23, match pattern 3
#### At position   25, match pattern 1
#### At position   26, match pattern 2


 
