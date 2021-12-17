#include <stdio.h>
#include <stdlib.h>

void readmatrix(int p, int pid, int N, int d, float **points, char* filename)
{

    FILE *pf;
    pf = fopen (filename, "r");
    if (pf == NULL)
        return;

    float *garbage = (float *)malloc(sizeof(float));
    for(int i = 0; i < N; i++)
    {
        for(int j = 0; j < d; j++){
            if(i >= pid * (N/p) && i < (pid + 1) * N/p){
                fscanf(pf, "%f", &points[i - pid * N/p][j]);
            }
            else{
                fscanf(pf, "%f", garbage);
            }
        }
    }
    fclose (pf); 
}

/*
void main(int argc, char **argv){
    int p = atoi(argv[1]);
    int N = atoi(argv[2]);
    int d = atoi(argv[3]);
    
    float **points = (float **)malloc(N/p * sizeof(float *));
    for(int i = 0;i < N/p;i++){
        points[i] = (float *)malloc(d * sizeof(float));
        for(int j = 0;j < d;j++){
            points[i][j] = 0;
        }
    }
    readmatrix(p, 4, N, d, points, argv[4]);
    for(int i = 0;i < N/p;i++){
        for(int j = 0;j < d;j++){
            printf("%f ", points[i][j]);
        }
        printf("\n");
    }
}
*/