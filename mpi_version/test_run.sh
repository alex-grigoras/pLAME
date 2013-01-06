#!/bin/bash
ulimit -s unlimited
module load  libraries/openmpi-1.6-gcc-4.4.6 
#module load compilers/solarisstudio12.3
#export OMP_NUM_THREADS=4
#collect -M OPENMPI mpiexec -np 2  -- ./mylame ../../Lame/wav_tests/Ultimate\ Spinach\ -\ Mind\ Flowers.wav
echo "MPI = 2" >&2
time mpirun -np 2 ./mylame ../wav_tests/Ultimate\ Spinach\ -\ Mind\ Flowers.wav
echo "MPI = 4" >&2
time mpirun -np 4 ./mylame ../wav_tests/Ultimate\ Spinach\ -\ Mind\ Flowers.wav
echo "MPI = 8" >&2
time mpirun -np 8 ./mylame ../wav_tests/Ultimate\ Spinach\ -\ Mind\ Flowers.wav
echo "Serial" >&2
time ../serial_version/mylame ../wav_tests/Ultimate\ Spinach\ -\ Mind\ Flowers.wav
