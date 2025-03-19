#include <stdbool.h>
#include "helper_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
int ** solution;


int ** allocateBoard(int size) {
    int *row = (int*)calloc(size *size, sizeof(int)); 

    int **board = (int**)calloc(size, sizeof(int*));
    for (int i = 0; i < size; i++) {
        board[i] = row + size * i;
    }
    return board; 
}

int ** copyBoard(int size, int **ogboard){
    int *row = (int*)calloc(size *size, sizeof(int)); 

    int **board = (int**)calloc(size, sizeof(int*));
    for (int i = 0; i < size; i++) {
        board[i] = row + size * i;
    }

    for (int i = 0; i <size; i ++){
        for(int j = 0; j <size; j ++){
            board[i][j] = ogboard[i][j];
        }
    }

    return board;
}

bool validateBoard(int ** board, int x, int y, int boardSize, int value)
{
    bool result = true; 
    #pragma omp parallel
    {
        #pragma omp task shared(result)
        {
            if(duplicateNumbersinRow(board, x, boardSize, value))
            {
                result = false; 
            }

        }
        #pragma omp task shared(result)
        {
            if(duplicateNumbersinCol(board,y, boardSize, value))
            {
                
                result = false; 
            }
        }
        #pragma omp task shared(result)
        {
            if(duplicateNumbersinBox(board,x,y, boardSize, value)){
                
                result = false; 
            }
        }
        
        #pragma omp taskwait
    }

    return result;
    
}

bool solve(int **board, index_t *unAssignInd, int NunAssign, int *values, int sideSize, int listsize, int depth)
{
    //printf("Thread %d executing solve()\n", omp_get_thread_num());
    int i = 0; 
    if(NunAssign >= listsize )
    {
        solution = board; 
        return true; 
    }
    
        int indexx=unAssignInd[NunAssign].x;
        int indexy=unAssignInd[NunAssign].y;

        for(int i = 0; i < sideSize; i++){
            //printf("index %d %d", indexx, indexy);
            int value = values[i];
            
            if(validateBoard(board, indexx, indexy, sideSize*sideSize, value)==true){
                #pragma omp critical
                board[indexx][indexy]=value;
                
                
                    int **copyboard = copyBoard(sideSize, board); 
                    #pragma omp task shared(sol) firstprivate(copyboard)
                    {
                        
                        //printf("Thread %d creating a task\n", omp_get_thread_num());
                        
                        bool sol = solve(copyboard, unAssignInd, NunAssign+1, values, sideSize, listsize, depth); 
                        
                    }
                    #pragma omp critical
                    board[indexx][indexy] = 0;
                
            }      

        }
    return false;      
}

void cleanup (int **board){
    free(board[0]);
    free(board);
}

int** readBoard(const char *filename, int *sideSize) {
    FILE *file = fopen(filename, "rb"); // Open file in binary mode
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    // Read the base and side length (1-byte each)
    uint8_t base, side;
    fread(&base, sizeof(uint8_t), 1, file);
    fread(&side, sizeof(uint8_t), 1, file);

    printf("Base: %2d, Side Length: %2d\n", base, side);

    // Allocate memory for the board
    uint8_t size = side * side;
    *sideSize = side; 
    //*tside = (int)side;
    //*tside = side; 
    uint8_t *board = (uint8_t*)malloc(size * sizeof(uint8_t));
    if (!board) {
        printf("Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    // Read the board numbers
    fread(board, sizeof(uint8_t), size, file);

    int **returnboard = allocateBoard(side); 
    //fread(returnboard[0], sizeof(uint8_t), size, file);

    
    // Print the board
    printf("Sudoku Board:\n");
    printf("size %d \n", size);
    for (uint8_t i = 0; i < size; i++) {
        printf("%2d ", board[i]);
        if ((i + 1) % side == 0) printf("\n"); // New row
    }
    for(int i =0; i< side; i++)
    {
        printf("\n");
        for(int j = 0; j < side; j ++){
            //printf("i %d j %d \n", i, j);
            returnboard[i][j] = (int)board[i*side+j]; 
            printf("%d ", returnboard[i][j]);
            
        }
    }
    printf("\n");

    // Cleanup
    free(board);
    fclose(file);
    return returnboard;
}

int main(int argc,char *argv[]){

    if(argc < 2)
    {
        printf("please provide a filename");
    }
    char * filename= argv[1];
    int *side;
    int *sideSize = calloc(1, sizeof(int));
    solution = readBoard(filename, sideSize);
    //int **board2 = readBoard(filename, sideSize);
    int* values = createValues(*sideSize);
    //printf("values %d %d %d %d %d %d %d %d %d \n", values[0], values[1],values[2],values[3], values[4],values[5],values[6],values[7],values[8]);
    int * listsize = calloc(1, sizeof(int));
    index_t * index = findEmptyCoord(solution, *sideSize, listsize); 
    for(int i = 0; i < 50; i++){
        //printf("x %d and y %d \n", index[i].x, index[i].y);
    }
    printf("running solve \n");

    /*int **board2 = allocateBoard(9);
    for (int i = 0; i < 9; i ++)
    {
        for(int j = 0; j<9; j++)
        {
            board2[i][j] = board[i][j];
        }
    }*/
    
    /*#pragma omp parallel num_threads(3)
    {
            //#pragma omp master
            //{


                int threadId = omp_get_thread_num();
                solve(board, index, threadId, values, *sideSize, *listsize);
            //}  
    }*/

    #pragma omp parallel
    {
        printf("Thread %d entering parallel region\n", omp_get_thread_num());
                    
            #pragma omp single
            {
                    printf("Thread %d executing single\n", omp_get_thread_num());

                    solve(solution, index, 0, values, *sideSize, *listsize, 0);
            }
    }

    for(int i =0; i< *sideSize; i++)
    {
        printf("\n");
        for(int j = 0; j < *sideSize; j ++){
            //printf("i %d j %d \n", i, j);
            //board[i][j] = board[i*9+j]; 
            printf("%d ", solution[i][j]);
            
        }
    }
    printf("\n");

    cleanup(solution);

}