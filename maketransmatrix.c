#include <stdio.h>
#include <stdlib.h>
//function used to make the Transaction Matrix
//Transaction Matrix is a numofprocesses^2 matrix where TM(i,j) = number of elements to be sent from i to j process
void maketransmatrix(int **tmat, int *numofsmalls, int p, int N){
    int i, j;
    int *tempsmalls = (int *)malloc(p * sizeof(int));
    int *tempbigs = (int *)malloc(p * sizeof(int));
    for(i = 0;i < p;i++) tempsmalls[i] = numofsmalls[i];
    for(i = 0;i < p;i++) tempbigs[i] = N/p - numofsmalls[i];
    
    //initialize all matrix malues to zero
    for(i = 0;i < p;i++){
        for(j = 0;j < p;j++) tmat[i][j] = 0;
    }
    //UPPER RIGHT QUARTER OF THE TMATRIX
    i = 0, j = p/2;
    while(i < p/2 && j < p){
        if(tempbigs[i] == tempsmalls[j]){
            int val = tempbigs[i];
            tmat[i][j] = val;
            tempbigs[i] -= val;
            tempsmalls[j] -= val;
            i++;
            j++;
        }
        else if(tempbigs[i] < tempsmalls[j]){
            int val = tempbigs[i];
            tmat[i][j] = val;
            tempbigs[i] -= val;
            tempsmalls[j] -= val;
            i++;
        }
        else {
            int val = tempsmalls[j];
            tmat[i][j] = val;
            tempbigs[i] -= val;
            tempsmalls[j] -= val;
            j++;
        }
    }
    //RESTORE TEMP VALUES
    for(i = 0;i < p;i++) tempsmalls[i] = numofsmalls[i];
    for(i = 0;i < p;i++) tempbigs[i] = N/p - numofsmalls[i];
    //LOWER LEFT QUARTER OF THE TMATRIX
    i = p/2, j = 0;
    while(i < p && j < p/2){
        if(tempbigs[j] == tempsmalls[i]){
            int val =  tempbigs[j];
            tmat[i][j] = val;
            tempbigs[j] -= val;
            tempsmalls[i] -= val;
            i++;
            j++;
        }
        else if(tempbigs[j] < tempsmalls[i]){
            int val =  tempbigs[j];
            tmat[i][j] = val;
            tempbigs[j] -= val;
            tempsmalls[i] -= val;
            j++;
        }
        else{
            int val = tempsmalls[i];
            tmat[i][j] = val;
            tempbigs[j] -= val;
            tempsmalls[i] -= val;
            i++;
        }
    }
    free(tempsmalls);
    free(tempbigs);
}