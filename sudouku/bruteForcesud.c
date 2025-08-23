#include <stdbool.h>
#include "helper_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/time.h>
#include <omp.h>

int ** solution;
bool solved = false; 

static double get_wall_seconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double seconds = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
    return seconds;
}

int** allocateBoard(int size) {
    int *row = (int*)calloc((size_t)(size * size), sizeof(int));
    int **board = (int**)calloc((size_t)size, sizeof(int*));

    if (!row || !board) {
        printf("Memory allocation failed\n");
        free(row);
        free(board);
        return NULL;
    }

    #pragma omp parallel for
    for (int i = 0; i < size; i++) {
        board[i] = row + size * i;
    }

    return board;
}

int** copyBoard(int size, int **src) {
    size_t boardSize = (size_t)size;

    int *row = (int*)calloc(boardSize * boardSize, sizeof(int));
    int **board = (int**)calloc(boardSize, sizeof(int*));

    if (!row || !board) {
        printf("Memory allocation failed\n");
        free(row);
        free(board);
        return NULL;
    }

    #pragma omp parallel for
    for (int i = 0; i < size; i++) {
        board[i] = &row[i * size];
    }

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            board[i][j] = src[i][j];
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

bool solve(int **board, index_t *unAssignInd, int NunAssign, int *values, int sideSize, int listsize, int depth, int depthvariable)
{
    if(solved){return true;}
    
    if(NunAssign >= listsize )
    {
        solved = true;
        //make sure the latest board is returned
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
                    //no need for critical as only one thread updates the specific index at a time
                    board[indexx][indexy]=value;
                        bool sol;
                        //avoid creating a new task for every cell -> workload too small
                        if(depth < depthvariable)
                        {
                            sol = solve(board, unAssignInd, NunAssign+1, values, sideSize, listsize, depth+1, depthvariable);
                           
                        }
                        else
                        {  
                            int **copyboard = copyBoard(sideSize, board);  
                            #pragma omp task shared(sol) firstprivate(copyboard)
                            {
                                //printf("Thread %d creating a task\n", omp_get_thread_num());
                                sol = solve(copyboard, unAssignInd, NunAssign+1, values, sideSize, listsize, 0, depthvariable); 
                                
                            }
                            
                        }
                        if(sol){
                            return true; 
                        }
                        board[indexx][indexy] = 0;
                          
                }      
        }
        
    return false;      
}

void cleanup (int **board){
    free(board[0]);
    free(board);
}


int** readtxt(const char *filename, int *sideSize) {
    FILE *file = fopen(filename, "r"); 
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

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

void convert_dat_to_txt(const char *input_file, const char *output_file) {
    FILE *f = fopen(input_file, "rb");
    if (!f) {
        perror("Error opening input file");
        return;
    }

    unsigned char base, side;
    if (fread(&base, 1, 1, f) != 1 || fread(&side, 1, 1, f) != 1) {
        fprintf(stderr, "Error reading base or side length\n");
        fclose(f);
        return;
    }

    printf("Base: %d, Side Length: %d\n", base, side);

    int size = side * side;
    unsigned char *board = (unsigned char *)malloc(size);
    if (!board) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(f);
        return;
    }

    if (fread(board, 1, size, f) != (size_t)size) {
        fprintf(stderr, "Warning: File size does not match expected board size!\n");
        free(board);
        fclose(f);
        return;
    }

    fclose(f);

    FILE *txt = fopen(output_file, "w");
    if (!txt) {
        perror("Error opening output file");
        free(board);
        return;
    }

    fprintf(txt, "%d\n\n", side);

    for (int i = 0; i < side; i++) {
        for (int j = 0; j < side; j++) {
            fprintf(txt, "%d ", board[i * side + j]);
        }
        fprintf(txt, "\n");
    }

    fclose(txt);
    free(board);

    printf("Converted %s -> \n", input_file, output_file);
}


int main(int argc,char *argv[]){

    if(argc < 3)
    {
        printf("please provide a filename and the number of tasks");
    }
    char * filename= argv[1];
    int tasks = atoi(argv[2]);
    int *sideSize = calloc(1, sizeof(int));
    //solution = readBoard(filename, sideSize);
    //int ** board = readBoard(filename, sideSize);

    char *outputfile = "txt.txt";
    convert_dat_to_txt(filename, outputfile);
    solution = readtxt(outputfile, sideSize);
    int* values = createValues(*sideSize);
    int * listsize = calloc(1, sizeof(int));
    index_t * index = findEmptyCoord(solution, *sideSize, listsize); 
    printf("listsize %d", *listsize);
    
    printf("running solve \n");
  
    //start parallel section
    double start = get_wall_seconds(); 
    #pragma omp parallel
    {
        printf("Thread %d entering parallel region\n", omp_get_thread_num());
            //only let one thread make the initial solve call      
            #pragma omp single
            {
                    printf("Thread %d executing single\n", omp_get_thread_num());
                    solve(solution, index, 0, values, *sideSize, *listsize,0, tasks);
                    
            }
    }
    double end = get_wall_seconds(); 
    printf("Time %f", end-start);
    //print board
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