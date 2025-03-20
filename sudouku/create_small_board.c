#include <stdio.h>
#include <stdint.h>

int main() {
    FILE *file = fopen("sudoku.dat", "wb");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    // Base and Side length for a 9x9 Sudoku
    uint8_t base = 3;
    uint8_t side = 9;

    // Solvable 9x9 Sudoku board (0 represents empty cells)
    uint8_t board[9][9] = {
        {5, 3, 4, 6, 7, 8, 9, 1, 0},
        {6, 7, 2, 1, 9, 5, 0, 0, 8},
        {1, 9, 8, 3, 0, 2, 0, 6, 7},
        {8, 0, 0, 0, 6, 0, 0, 2, 3},
        {4, 0, 6, 8, 0, 3, 0, 9, 1},
        {7, 0, 0, 9, 2, 4, 8, 0, 6},
        {9, 6, 0, 0, 0, 0, 2, 8, 0},
        {2, 8, 7, 4, 1, 9, 6, 3, 5},
        {0, 4, 5, 2, 8, 0, 0, 7, 9}
    };

    // Write the base and side length
    fwrite(&base, sizeof(uint8_t), 1, file);
    fwrite(&side, sizeof(uint8_t), 1, file);

    // Write the board data
    fwrite(board, sizeof(uint8_t), 9 * 9, file);

    fclose(file);
    printf("sudoku.dat (solvable 9x9) created successfully!\n");
    return 0;
}
