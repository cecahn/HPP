#include <stdbool.h>
#include "helper_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

int ** solution;
bool solved = false; 


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

bool solve_seq(int **board, index_t *unAssignInd, int NunAssign, int *values, int sideSize, int listsize){
    
    if(NunAssign == listsize )
    {
        //solution = board; 
        return true; 
    }
        int indexx=unAssignInd[NunAssign].x;
        int indexy=unAssignInd[NunAssign].y;
        for(int i = 0; i < sideSize; i++){
            //printf("index %d %d", indexx, indexy);
            int value = values[i];
            
            if(validateBoard(board, indexx, indexy, sideSize*sideSize, value)==true){

                board[indexx][indexy]=value;

                bool sol = solve_seq(board, unAssignInd, NunAssign+1, values, sideSize, listsize);
                if(sol){
                    return true; 
                }
                
            } 
            board[indexx][indexy] = 0;     

        }
    return false; 
}

bool solve(int **board, index_t *unAssignInd, int NunAssign, int *values, int sideSize, int listsize, int depth)
{
    if(solved){return true;}
    //printf("Thread %d executing solve()\n", omp_get_thread_num());
    //int i = 0; 
    
    if(NunAssign >= listsize )
    {
        solved = true;
        solution = board;
        return true;  
    }
    
        int indexx=unAssignInd[NunAssign].x;
        int indexy=unAssignInd[NunAssign].y;

            for(int i = 0; i < sideSize; i++){
                //printf("index %d %d", indexx, indexy);
                int value = values[i];
                
                if(validateBoard(board, indexx, indexy, sideSize*sideSize, value)==true){
                    //#pragma omp critical
                    board[indexx][indexy]=value;
                    
                        bool sol;
                        //avoid creating a new task for every cell -> workload too small
                        if(depth < 25)
                        {
                            sol = solve(board, unAssignInd, NunAssign+1, values, sideSize, listsize, depth+1);
                           
                        }
                        else
                        {  
                            int **copyboard = copyBoard(sideSize, board);  
                            #pragma omp task shared(sol) firstprivate(copyboard)
                            {
                                //printf("Thread %d creating a task\n", omp_get_thread_num());
                                sol = solve(copyboard, unAssignInd, NunAssign+1, values, sideSize, listsize, 0); 
                                
                                if(sol){
                                    //#pragma omp taskwait
                                }
                            }
                            //cleanup(copyboard);
                        }
                        if(sol){
                            return true; 
                        }
                        board[indexx][indexy] = 0;
                        /*if (!sol){
                            //solution = board;
                            board[indexx][indexy] = 0;
                        }*/
                        
                        //#pragma omp critical
                          
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

void onlyOneValid(int **board, int x, int y, int* values, int size)
{
    for(int i= 0; i < size; i++){
        if(validateBoard(board, x, y, size*size, values[i]))
        {
            for(int j = i+1; j<size; j++)
            {
                if(validateBoard(board, x, y,size, values[j] ))
                {
                    return;
                }
            }
            printf("found a coordinate with only one valid solution");
            board[x][y] = values[i];
            return; 
            
        }
    }
}

int** readtxt(const char *filename, int *sideSize) {
    FILE *file = fopen(filename, "r"); // Open in text mode
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    // Read side length from the file
    if (fscanf(file, "%d", sideSize) != 1) {
        printf("Invalid file format\n");
        fclose(file);
        return NULL;
    }

    int side = *sideSize;
    printf("Side Length: %d\n", side);

    // Allocate memory for the board
    int **board = allocateBoard(side);
    if (!board) {
        printf("Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    // Read board numbers from the file
    for (int i = 0; i < side; i++) {
        for (int j = 0; j < side; j++) {
            if (fscanf(file, "%d", &board[i][j]) != 1) {
                printf("Invalid board data\n");
                fclose(file);
                return NULL;
            }
        }
    }

    fclose(file);

    // Print the board
    printf("Sudoku Board:\n");
    for (int i = 0; i < side; i++) {
        for (int j = 0; j < side; j++) {
            printf("%2d ", board[i][j]);
        }
        printf("\n");
    }

    return board;
}
int main(int argc,char *argv[]){

    if(argc < 2)
    {
        printf("please provide a filename");
    }
    char * filename= argv[1];
    int *side;
    int *sideSize = calloc(1, sizeof(int));
    //solution = readBoard(filename, sideSize);
    //int ** board = readBoard(filename, sideSize);
    solution = readtxt(filename, sideSize);
    int* values = createValues(*sideSize);
    int * listsize = calloc(1, sizeof(int));
    index_t * index = findEmptyCoord(solution, *sideSize, listsize); 
    printf("listsize before %d", *listsize);

    for(int i = 0; i < 51; i++){
        onlyOneValid(solution, index[i].x, index[i].y, values, (*sideSize));
    }
    
    index = findEmptyCoord(solution, *sideSize, listsize); 
    printf("listsize after %d", *listsize);
    for(int i = 0; i < 50; i++){
        //printf("x %d and y %d \n", index[i].x, index[i].y);
    }
    printf("running solve \n");
  
    //solve_seq(solution, index, 0, values, *sideSize, *listsize);

    #pragma omp parallel
    {
        printf("Thread %d entering parallel region\n", omp_get_thread_num());
                    
            #pragma omp single
            {
                    printf("Thread %d executing single\n", omp_get_thread_num());

                    solve(solution, index, 0, values, *sideSize, *listsize,0);
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

    /*for(int i =0; i< *sideSize; i++)
    {
        printf("\n");
        for(int j = 0; j < *sideSize; j ++){
            //printf("i %d j %d \n", i, j);
            //board[i][j] = board[i*9+j]; 
            printf("%d ", board[i][j]);
            
        }
    }*/

    cleanup(solution);
    //cleanup(board);

}