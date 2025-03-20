#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include "helper_functions.h"
#include <stdio.h>
#include <omp.h>



bool duplicateNumbersinRow(int **board, int x, int size, int value)
{
    int side = sqrt(size); 
    for(int i = 0; i < side; i ++){
        //if(board[x][i] != 0)
        //{       
            if( board[x][i] == value)
            {
                        //printf(" duplicate numbers in row returning true %d %d, i = %d, j = %d \n", board[x][i], board[x][j], i, j);
                        return true; 
            }
                           
        //}
            
    }
    return false; 
}

bool duplicateNumbersinCol(int **board, int y, int size, int value)
{
    int side = sqrt(size); 
    for(int i = 0; i < side; i ++){
        //if(board[i][y] != 0)
        //{
            
            if( board[i][y] == value)
            {
                        //printf(" duplicate numbers in col returning true %d %d, i = %d, j = %d \n", board[i][y], board[j][y], i, j);
                    return true; 
            }
                          
        //} 
    }  
    
    return false; 
}

int calcIndex(int coordinate, int boxsize, int size)
{
    if (coordinate == 0){return 0;} 
    //printf("coordinate %d", coordinate); 
    int modsize =coordinate % size; 
    int modbox = coordinate % boxsize; 
    //printf("modsize %d modbox %d", modsize, modbox);
    if(modbox == 0 || modsize == 0)
    {
        return coordinate; 
    }
    else {
        return (coordinate - modbox); 
    }
}

bool duplicateNumbersinBox(int **board, int x, int y, int size, int value){
    int boxSize = sqrt(sqrt(size));
    int newx = calcIndex(x, boxSize, boxSize);
    int newy = calcIndex(y, boxSize, boxSize);
    //printf("newx %d newy %d \n", newx, newy);
    //printf("boxSize %d", boxSize);
    for(int i = newx; i < boxSize+newx; i ++){
        for(int j =newy; j < boxSize+newy; j ++){
            if(board[i][j]!=0)
            {
                if(board[i][j] == value){
                    //printf("duplicate numbers in box returning true %d %d for newx %d newy %d, and i %d, j %d\n",board[x][y], board[i][j], x, y, i, j); 
                    return true; 
                }
            }
            
        }
    }
    
    return false;
    
     
    
}

index_t *findEmptyCoord(int **board, int side, int *listsize) {
    index_t *index = calloc(side * side, sizeof(index_t));  // Allocate max possible
    if (!index) {
        perror("Memory allocation failed");
        return NULL;
    }

    int place = 0;
    //#pragma omp parallel
    {
        index_t *local_index = calloc(side * side, sizeof(index_t));  // Thread-local storage
        int local_place = 0;

        //#pragma omp for collapse(2)
        for (int i = 0; i < side; i++) {
            for (int j = 0; j < side; j++) {
                if (board[i][j] == 0) {
                    local_index[local_place].x = i;
                    local_index[local_place].y = j;
                    local_place++;
                }
            }
        }

        //#pragma omp critical
        {
            for (int k = 0; k < local_place; k++) {
                index[place++] = local_index[k];
            }
        }

        free(local_index);
    }

    *listsize = place;
    return index;
}

int * createValues(int side){
    int *values = calloc(side, sizeof(int)); 

    #pragma omp parallel for num_threads(side)
    for(int i = 1; i < side+1; i ++)
    {
        values[i - 1] = i; 
    }
    return values; 
}