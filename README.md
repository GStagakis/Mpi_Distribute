# Mpi_Distribute

Linux Execution:

mpirun -np numberofprocesses ./point_distributing numberofpoints numberofdimensions datfile

If no datfile is given the algorithm works for random points

Number of processes must be even

Number of points must be divisible by the number of processes

!!!Warning: Algorithm may fail if it finds more than one distances equal to median(Prints Error Message).
  
