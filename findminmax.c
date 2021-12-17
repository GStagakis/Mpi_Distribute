//C function to find min and max
#include <stdio.h>
#include <stdlib.h>

void findminmax(float *minmax, float *distances, int numofdistances){
    minmax[0] = minmax[1] = distances[0];
    for(int i = 1; i < numofdistances; i++)
    {
        if(minmax[0] > distances[i])  minmax[0] = distances[i];   
		if(minmax[1] < distances[i])  minmax[1] = distances[i];       
    }
}

//C function to verify the algorithm by checking minmax table
void verify(float **minmax, int p){
    for(int i = 1;i < p;i++){
        if(minmax[i][0] < minmax[i-1][1]){      //if the min of the process is smaller than the max of the last process...
            printf("ALGORITHM FAILED!\n");
            return;
        }
    }
}
