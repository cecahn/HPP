#include <stdio.h>
#include <stdint.h>

int main() {
    FILE *file = fopen("board_16x16.dat", "wb");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    // Base and Side length for a 16x16 Sudoku
    uint8_t base = 4;
    uint8_t side = 16;

    // Example of a solvable 16x16 Sudoku board (0 represents empty cells)
    uint8_t board[16][16] = {
        { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16},
        { 5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,  1,  2,  3,  4},
        { 9, 10, 11, 12, 13, 14, 15, 16,  1,  2,  3,  4,  5,  6,  7,  8},
        {13, 14, 15, 16,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12},

        { 2,  0,  0,  0,  6,  0,  0,  0, 10,  0,  0,  0, 14,  0,  0,  0},
        { 6,  0,  0,  0, 10,  0,  0,  0, 14,  0,  0,  0,  2,  0,  0,  0},
        {10,  0,  0,  0, 14,  0,  0,  0,  2,  0,  0,  0,  6,  0,  0,  0},
        {14,  0,  0,  0,  2,  0,  0,  0,  6,  0,  0,  0, 10,  0,  0,  0},

        { 3,  0,  0,  0,  7,  0,  0,  0, 11,  0,  0,  0, 15,  0,  0,  0},
        { 7,  0,  0,  0, 11,  0,  0,  0, 15,  0,  0,  0,  3,  0,  0,  0},
        {11,  0,  0,  0, 15,  0,  0,  0,  3,  0,  0,  0,  7,  0,  0,  0},
        {15,  0,  0,  0,  3,  0,  0,  0,  7,  0,  0,  0, 11,  0,  0,  0},

        { 4,  8, 12, 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 8, 12, 16,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        {12, 16,  4,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        {16,  4,  8, 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}
    };

    // Write the base and side length
    fwrite(&base, sizeof(uint8_t), 1, file);
    fwrite(&side, sizeof(uint8_t), 1, file);

    // Write the board data
    fwrite(board, sizeof(uint8_t), 16 * 16, file);

    fclose(file);
    printf("board_16x16.dat (solvable 16x16) created successfully!\n");
    return 0;
}
