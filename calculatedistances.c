#include <stdio.h>
#include <stdlib.h>
//function used to calculate the distances of given points from a given pivot
void calculatedistances(float *distances, float **points, int numofpoints, int dimensions, float *pivot){
    for(int i = 0;i < numofpoints;i++){
        distances[i] = 0;
        for(int j = 0;j < dimensions;j++){
            distances[i] += (points[i][j] - pivot[j]) * (points[i][j] - pivot[j]);        //use square instead of actual distance
        }
    }
}