#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 25  // 25x25 Sudoku
#define EMPTY 0

typedef struct {
    int size;
    int **grid;
} SudokuBoard;

int solved = 0;

// Function to allocate memory for the board
SudokuBoard* createBoard(int size) {
    SudokuBoard *board = (SudokuBoard*) malloc(sizeof(SudokuBoard));
    board->size = size;
    board->grid = (int**) malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++) {
        board->grid[i] = (int*) malloc(size * sizeof(int));
    }
    return board;
}

// Function to free board memory
void freeBoard(SudokuBoard *board) {
    for (int i = 0; i < board->size; i++) {
        free(board->grid[i]);
    }
    free(board->grid);
    free(board);
}

// Function to copy board
SudokuBoard* copyBoard(SudokuBoard *src) {
    SudokuBoard *newBoard = createBoard(src->size);
    for (int i = 0; i < src->size; i++) {
        for (int j = 0; j < src->size; j++) {
            newBoard->grid[i][j] = src->grid[i][j];
        }
    }
    return newBoard;
}

// Check if a number can be placed at board[row][col]
int isValid(SudokuBoard *board, int row, int col, int num) {
    for (int i = 0; i < board->size; i++) {
        if (board->grid[row][i] == num || board->grid[i][col] == num) {
            return 0;
        }
    }
    
    // Check 5x5 region
    int regionSize = 5;
    int boxRow = (row / regionSize) * regionSize, boxCol = (col / regionSize) * regionSize;
    for (int i = 0; i < regionSize; i++) {
        for (int j = 0; j < regionSize; j++) {
            if (board->grid[boxRow + i][boxCol + j] == num) {
                return 0;
            }
        }
    }
    return 1;
}

// Sequential Brute-force Solver
void solve_bruteforce_seq(SudokuBoard *board, int row, int col) {
    if (solved) return;

    if (row == N) {
        solved = 1;
        return;
    }

    int nextRow = (col == N - 1) ? row + 1 : row;
    int nextCol = (col == N - 1) ? 0 : col + 1;

    if (board->grid[row][col] != EMPTY) {
        solve_bruteforce_seq(board, nextRow, nextCol);
        return;
    }

    for (int num = 1; num <= N; num++) {
        if (isValid(board, row, col, num)) {
            board->grid[row][col] = num;
            solve_bruteforce_seq(board, nextRow, nextCol);
            if (solved) return;
            board->grid[row][col] = EMPTY;
        }
    }
}

// Parallel Brute-force Solver
void solve_bruteforce_par(SudokuBoard *board, int row, int col, int depth) {
    if (solved) return;

    if (row == N) {
        solved = 1;
        return;
    }

    int nextRow = (col == N - 1) ? row + 1 : row;
    int nextCol = (col == N - 1) ? 0 : col + 1;

    if (board->grid[row][col] != EMPTY) {
        solve_bruteforce_par(board, nextRow, nextCol, depth + 1);
        return;
    }

    for (int num = 1; num <= N; num++) {
        if (isValid(board, row, col, num)) {
            if (depth > N) {
                board->grid[row][col] = num;
                solve_bruteforce_seq(board, nextRow, nextCol);
                board->grid[row][col] = EMPTY;
            } else {
                SudokuBoard *newBoard = copyBoard(board);
                newBoard->grid[row][col] = num;

                #pragma omp task default(none) firstprivate(newBoard, nextRow, nextCol, depth)
                solve_bruteforce_par(newBoard, nextRow, nextCol, depth + 1);

                freeBoard(newBoard);
            }
        }
    }
}

SudokuBoard* readtxt(const char *filename, int *sideSize) {
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
    SudokuBoard *board = createBoard(side);

    // Read board numbers from the file
    for (int i = 0; i < side; i++) {
        for (int j = 0; j < side; j++) {
            if (fscanf(file, "%d", board->grid[i][j]) != 1) {
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
            printf("%2d ", board->grid[i][j]);
        }
        printf("\n");
    }

    return board;
}

int main() {
    int * sidesize;
    SudokuBoard *board = readtxt("25x25.txt", sidesize);

    // Initialize board (set some empty cells)
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            board->grid[i][j] = EMPTY; // Start with an empty board for testing

    // Parallel execution
    #pragma omp parallel
    {
        #pragma omp single
        solve_bruteforce_par(board, 0, 0, 0);
    }

    if (solved) {
        printf("Sudoku Solved!\n");
    } else {
        printf("No solution found.\n");
    }

    freeBoard(board);
    return 0;
}




