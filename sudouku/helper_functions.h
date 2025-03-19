#pragma once

typedef struct {
    int x;
    int y;
} index_t;

bool duplicateNumbersinRow(int **board, int x, int size, int value);

bool duplicateNumbersinCol(int **board, int y, int size, int value);

bool duplicateNumbersinBox(int **board, int x, int y, int size, int value);

index_t *findEmptyCoord(int ** board, int side, int *listsize);

int * createValues(int side);