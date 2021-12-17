#include <stdio.h>
#include <unistd.h>     //getpid for random number generator
#include <stdlib.h>     //atoi
#include <math.h>       //pow
#include <mpi.h>
#include "calculatedistances.c"
#include "quickselect.c"
#include "alloc_2d_int.c"
#include "alloc_2d_float.c"
#include "maketransmatrix.c"
#include "findminmax.c"
#include "readmatrix.c"
#include <time.h>

//HELPER FUNCTIONS
void printPoints(int numofpoints,  int d, float **points, int pid){     //process prints its name and points
    printf("Process: %d\n", pid);   
    for(int i = 0; i < numofpoints; i++){
        for(int j = 0; j < d; j++){
            printf("%.2f ", points[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}
void print2D(int M, int N, int **matrix, int pid){
    printf("PID%d:\n", pid);
    for(int i = 0;i < M;i++){
        for(int j = 0;j < N;j++){
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}
void printdistances(int numofpoints, float *distances, int pid){
    printf("Distances of Process: %d\n", pid);
    for(int i = 0;i < numofpoints;i++) printf("%.2f ", distances[i]);
    printf("\n");
} 
void printfloatarr(int numofpoints, float *distances, int pid, char *str){
    printf("PID:%d %s\n", pid, str);
    for(int i = 0;i < numofpoints;i++) printf("%.2f ", distances[i]);
    printf("\n");
}  
void printnumofsmalls(int numofpoints, int *distances, int pid){
    printf("PID%d: found numofsmalls:\n", pid);
    for(int i = 0;i < numofpoints;i++) printf("%d ", distances[i]);
    printf("\n");
}  
//MY FUNCTION FOR PARTITION USED TO FIND SMALLER AND BIGGER VALUES THAN MEDIAN IN EACH PROCESS
//partitions distances and points with given pivot
//returns number of smaller or equal values
int mypartition(float arr[], int l, int r, float pivot, float **points, int d)
{
    int i = l;
    for (int j = l; j <= r; j++) {
        if (arr[j] <= pivot) {
            swap(&arr[i], &arr[j]);
            for(int k = 0;k < d;k++){
                swap(&points[i][k], &points[j][k]);
            }
            i++;
        }
    }
    return i;
}


void distributeByMedian(int p, int pid, int N, int d, float **points, int pstartid, int pendid, float *pivot){
    int i, j;
    float *distances = (float *)malloc(N/p * sizeof(float));
    float *floatsendbuf, *floatrecvbuf;
    int *intsendbuf, *intrecvbuf;
    MPI_Status status;
    MPI_Request request;
    int numofsmalls;

    if(pid == pstartid){
        //calculate distances from pivot
        calculatedistances(distances, points, N/p, d, pivot);
        
        //receive distances from all other processes
        floatrecvbuf = (float *)malloc(N * sizeof(float));
        for(i = 0;i  < N/p;i++) floatrecvbuf[i] = distances[i];
        for(i = 1;i < p;i++){
            MPI_Recv(floatrecvbuf + i * N/p, N/p, MPI_FLOAT, pstartid + i, 10 + pstartid + i, MPI_COMM_WORLD, &status);
        }
        
        float median = quickselect(floatrecvbuf, 0, N-1, N/2);      //use quickselect to find median
        free(floatrecvbuf);
        //printf("PID%d found median to be:%.2f\n", pid, median);
        floatsendbuf = &median;
        //send median to all other processes
        for(i = 1;i < p;i++){
            MPI_Send(floatsendbuf, 1, MPI_FLOAT, pstartid + i, 20 + pstartid + i, MPI_COMM_WORLD);
        }

        numofsmalls = mypartition(distances, 0, N/p - 1, median, points, d);       //partition and calculation of number of distances smaller than median
        
        //receive numofsmalls from all other processes
        intrecvbuf = (int *)malloc(p * sizeof(int));
        intrecvbuf[0] = numofsmalls;
        for(i = 1;i < p;i++){
            MPI_Recv(intrecvbuf + i, 1, MPI_INT, pstartid + i, 30 + pstartid + i, MPI_COMM_WORLD, &status);
        }


        //*****************************
        //CHECK FOR MEDIAN MULTIPLICITY
        int totalnumofsmalls = 0;
        for(i = 0;i < p;i++) totalnumofsmalls += intrecvbuf[i];
        if(totalnumofsmalls > N/2){
            printf("FOUND MORE THAN ONE MEDIANS\n");
        }
        //*****************************
        

        //callfunction maketransmatrix to make the transaction matrix for all processes
        int **tmat = alloc_2d_int(p, p);
        maketransmatrix(tmat, intrecvbuf, p, N);
        free(intrecvbuf);
        //print2D(p, p, tmat, pid);
        //send sends/receives of each process to each process
        for(i = 1;i < p;i++){
            intsendbuf = tmat[i];
            MPI_Send(intsendbuf, p, MPI_INT, pstartid + i, 40 + pstartid + i, MPI_COMM_WORLD);
        }
        
        //scan its "sends" in tmat
        int counter = 0;
        for(j = 0;j < p;j++){
            if(tmat[pid-pstartid][j] != 0){
                int numoftrans = tmat[pid-pstartid][j];
                float **sendedpoints = alloc_2d_float(numoftrans, d);
                float **recvedpoints = alloc_2d_float(numoftrans, d);
                sendedpoints = &points[numofsmalls + counter];    //send the bigs
                MPI_Isend(&(sendedpoints[0][0]), numoftrans * d, MPI_FLOAT, j + pstartid, pid*pid + (j + pstartid)*(j + pstartid), MPI_COMM_WORLD, &request);
                MPI_Recv(&(recvedpoints[0][0]), numoftrans * d, MPI_FLOAT, j + pstartid, pid*pid + (j + pstartid)*(j + pstartid), MPI_COMM_WORLD, &status);
                for(int k = 0;k < numoftrans;k++){
                    for(int l = 0;l < d;l++){
                        points[numofsmalls + counter + k][l] = recvedpoints[k][l];          //receive smalls
                    }
                }
                MPI_Wait(&request, &status);
                free(recvedpoints[0]);
                free(recvedpoints);
                //counter used to send points from the right spot in points matrix of the sending process and write received points in the right spot of points matrix(counts transactions so far)
                counter += numoftrans;
            }
        }
        free(tmat[0]);
        free(tmat);
    } else if(pid > pstartid && pid <= pendid){
        //calculate distances from pivot
        calculatedistances(distances, points, N/p, d, pivot);
        
        //send distances to master process
        floatsendbuf = distances;
        MPI_Send(floatsendbuf, (N/p), MPI_FLOAT, pstartid, 10 + pid, MPI_COMM_WORLD);

        //receive median from master process
        floatrecvbuf = (float *)malloc(sizeof(float));
        MPI_Recv(floatrecvbuf, 1, MPI_FLOAT, pstartid, 20 + pid, MPI_COMM_WORLD, &status);
        float median = *floatrecvbuf;

        numofsmalls = mypartition(distances, 0, N/p - 1, median, points, d);       //partition and calculation of number of distances smaller than median

        //send numofsmalls to master
        intsendbuf = &numofsmalls;
        MPI_Send(intsendbuf, 1, MPI_INT, pstartid, 30 + pid, MPI_COMM_WORLD);
        //receive sends/receives from master process
        intrecvbuf = (int *)malloc(p * sizeof(int));
        MPI_Recv(intrecvbuf, p, MPI_INT, pstartid, 40 + pid, MPI_COMM_WORLD, &status);
        
        //scan its "sends" in intrecvbuf
        int counter = 0;
        for(j = 0;j < p;j++){
            if(intrecvbuf[j] != 0){
                int numoftrans = intrecvbuf[j];
                float **sendedpoints = alloc_2d_float(numoftrans, d);
                float **recvedpoints = alloc_2d_float(numoftrans, d);
                if(pid < pstartid + p/2){
                    sendedpoints = &points[numofsmalls + counter];    //send bigs
                    MPI_Isend(&(sendedpoints[0][0]), numoftrans * d, MPI_FLOAT, j + pstartid, pid*pid + (j + pstartid)*(j + pstartid), MPI_COMM_WORLD, &request);
                    MPI_Recv(&(recvedpoints[0][0]), numoftrans * d, MPI_FLOAT, j + pstartid, pid*pid + (j + pstartid)*(j + pstartid), MPI_COMM_WORLD, &status);
                    for(int k = 0;k < numoftrans;k++){
                        for(int l = 0;l < d;l++){
                            points[numofsmalls + counter + k][l] = recvedpoints[k][l];          //receive smalls
                        }
                    }
                    MPI_Wait(&request, &status);
                }
                else {
                    sendedpoints = &points[counter];                  //otherwise send the smalls
                    MPI_Isend(&(sendedpoints[0][0]), numoftrans * d, MPI_FLOAT, j + pstartid, pid*pid + (j + pstartid)*(j + pstartid), MPI_COMM_WORLD, &request);
                    MPI_Recv(&(recvedpoints[0][0]), numoftrans * d, MPI_FLOAT, j + pstartid, pid*pid + (j + pstartid)*(j + pstartid), MPI_COMM_WORLD, &status);
                    for(int k = 0;k < numoftrans;k++){
                        for(int l = 0;l < d;l++){
                            points[counter + k][l] = recvedpoints[k][l];                        //receive bigs
                        }
                    }
                    MPI_Wait(&request, &status);
                }
                //free recv buffer
                free(recvedpoints[0]);
                free(recvedpoints);                               
                //counter used to send points from the right spot in points matrix of the sending process and write received points in the right spot of points matrix(counts transactions so far)
                counter += numoftrans;
            }
        }
        free(intrecvbuf);
    }
}
//function used to call distributeByMedian recursively
void calldistributeByMedian(int p, int pid, int N, int d, float **points, int pstartid, int pendid, float *pivot){
    if(p < 2) return;
    distributeByMedian(p, pid, N, d, points, pstartid, pendid, pivot);
    calldistributeByMedian(p/2, pid, N/2, d, points, pstartid, pstartid + p/2 - 1, pivot);    //call for left half
    calldistributeByMedian(p/2, pid, N/2, d, points, pstartid + p/2, pendid, pivot);    //call for right half
}


int main(int argc, char **argv){
    int p, pid;
    MPI_Init(&argc, &argv);     //Initialize communication

    MPI_Comm_size(MPI_COMM_WORLD, &p);      //Find communication size
    
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);        //Find process rank
    if(argc < 3 || atoi(argv[1]) == 0 || atoi(argv[2]) == 0 || p%2 !=0 || atoi(argv[1]) % p != 0){      //check validity of input arguments
        if(pid == 0)    printf("Error!!!\nUsage: mpirun -np <numberofprocesses> ./point_distributing <numberofpoints> <numberofdimensions>\nNumber of processes must be even\nNumber of points must be divisible by the number of processes\n");
        MPI_Finalize();
        return 0;
    }
    int N = atoi(argv[1]), d = atoi(argv[2]);     //p: number of processes    N:number of total points    d:number of point dimensions
    struct timespec start, finish;
    double elapsed;


    //getinput
    float **points = alloc_2d_float(N, d);      //allocates Nxd points matrix
    if(argc == 4) readmatrix(p, pid, N, d, points, argv[3]);
    else{
        srand((unsigned) getpid());      //Intializes random number generator
        for(int i = 0; i < N/p; i++){     //Initializes points matrix with random numbers
            for(int j = 0; j < d; j++){
                points[i][j] = rand() % 10 + (rand() % 10)/10.0;
            }
        }
    }

    //************************************************************************************************************************************//
    //TO DO...
    //pivot has to be standard for all processes.Process with pid 0 selects one of its points randomly.
    float *pivot = (float *)malloc(d * sizeof(float));
    if(pid == 0){
        int pivotindex = rand() % (N/p);
        for(int i = 0;i < d;i++) pivot[i] = points[pivotindex][i];
        //printfloatarr(d, pivot, pid, "SELECTED PIVOT TO BE:");
        //printf("\n");
    }
    MPI_Bcast(pivot, d, MPI_FLOAT, 0, MPI_COMM_WORLD);
    
    if(pid == 0) clock_gettime(CLOCK_MONOTONIC, &start);            //start clock
    calldistributeByMedian(p, pid, N, d, points, 0, p-1, pivot);
    //Verification
    float *distances = (float *)malloc(N/p * sizeof(float));
    calculatedistances(distances, points, N/p, d, pivot);
    //printdistances(N/p, distances, pid);
    MPI_Status status;
    if(pid == 0){
        float **minmax = alloc_2d_float(p, 2);
        findminmax(minmax[0], distances, N/p);
        for(int i = 1;i < p;i++){
            MPI_Recv(minmax[i], 2, MPI_FLOAT, i, 1000, MPI_COMM_WORLD, &status);
        }
        verify(minmax, p);
        clock_gettime(CLOCK_MONOTONIC, &finish);                        //end clock(by the time of verification all processes have their points sorted)
        elapsed = (finish.tv_sec - start.tv_sec);
        elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
        printf("%f\n", elapsed);
        //free points
        free(minmax[0]);
        free(minmax);
    }
    else{
        float *minmax = (float *)malloc(2 * sizeof(float));
        findminmax(minmax, distances, N/p);
        MPI_Send(minmax, 2, MPI_FLOAT, 0, 1000, MPI_COMM_WORLD);
    }
    free(distances);
    free(points[0]);
    free(points);
    MPI_Finalize();     //End communication
    return 0;
}