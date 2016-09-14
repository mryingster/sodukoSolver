// -*- compile-command: "gcc -std=c99 -o sudokuSolver sudokuSolver.c -Wall -lm" -*-
// Copyright (c) 2016 Michael Caldwell
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

typedef struct puzzle puzzle;

struct puzzle {
    int cell[9][9][10];
};

// Check if puzzle is valid.
bool isValidPuzzle(puzzle g) {
    // Check rows for duplicate values
    for (int x=0; x<9; x++)
        for (int y=0; y<8; y++)
            for (int z=y+1; z<9; z++)
                if (g.cell[x][y][0] != 0)
                    if (g.cell[x][y][0] == g.cell[x][z][0])
                        return false;

    // Check cols for duplicate values
    for (int y=0; y<9; y++)
        for (int x=0; x<8; x++)
            for (int z=x+1; z<9; z++)
                if (g.cell[x][y][0] != 0)
                    if (g.cell[x][y][0] == g.cell[z][y][0])
                        return false;

    // Check squares for duplicate values
    for (int qx=0; qx<9; qx+=3)
        for (int qy=0; qy<9; qy+=3)
        {
            int used[10] = {0};
            for (int x=0; x<3; x++)
                for (int y=0; y<3; y++)
                    used[g.cell[qx+x][qy+y][0]]++;

            for (int i=1; i<10; i++)
                if (used[i] > 1)
                    return false;
        }

    return true;
}

bool isSolved(puzzle g) {
    for (int x=0; x<9; x++)
        for (int y=0; y<9; y++)
            if (g.cell[x][y][0] == 0)
                return false;
    return true;
}

void printPuzzle(puzzle g) {
    printf("\n");
    for (int i=0; i<9; i++)
    {
        for (int j=0; j<9; j++)
        {
            // Print number or blank if value is 0
            if (g.cell[i][j][0] == 0)
                printf("  ");
            else
                printf(" %d", g.cell[i][j][0]);

            // Print vertical separator or newline
            if ((j+1) % 3 == 0 && j < 8)
                printf(" |");
            if (j == 8)
                printf("\n");
         }
        // Print horizontal separator
        if ((i + 1) % 3 == 0 && i < 8)
            printf("-------+-------+-------\n");
    }
    printf("\n");
}

void printPuzzleOptions(puzzle g) {
    printf("\n");
    for (int x=0; x<9; x++)
    {
        for (int y=0; y<9; y++)
        {
            if (g.cell[x][y][0] > 0)
                printf("%d ", g.cell[x][y][0]);
            else
            {
                printf("[");
                for (int i=1; i<10; i++)
                    if (g.cell[x][y][i] == true)
                        printf("%d, ", i);
                printf("]");
            }
        }
        printf("\n");
    }
    printf("\n");
}

int isSingleCandidate(puzzle *g, int x, int y) {
    int count = 0;
    int candidate = 0;
    for (int i=1; i<10; i++)
        if (g->cell[x][y][i] == false)
            count++;
        else
            candidate = i;

    if (count == 8)
        return candidate;
    return 0;
}

void eliminateNumberInLine(puzzle *g, int x, int y) {
    for (int i=0; i<9; i++)
    {
        if (g->cell[i][y][0] > 0)                     // Search in row. If not blank (0)...
            g->cell[x][y][g->cell[i][y][0]] = false;  // Mark candidate for that number as false
        if (g->cell[x][i][0] > 0)                     // Search in col. If not blank (0)...
            g->cell[x][y][g->cell[x][i][0]] = false;  // Mark candidate for that number as false
    }
}

void eliminateNumberInBox(puzzle *g, int x, int y) {
    int row = ceil(x/3)*3;                        // Confine to 3x3 square
    int col = ceil(y/3)*3;                        // Confine to 3x3 square

    for (int i=row; i<row+3; i++)
        for (int n=col; n<col+3; n++)                    // Search square
            if (g->cell[i][n][0] > 0)                    // for non-zero values
                g->cell[x][y][g->cell[i][n][0]] = false; // Mark false
}

void crossCheckBoxCandidates(puzzle *g, int x, int y) {
    int row = ceil(x/3)*3;
    int col = ceil(y/3)*3;

    for (int j=1; j<10; j++)                    // For each candidate for x, y
    {
        if (g->cell[x][y][j] == 0) continue;    // Skip non-candidates
        bool uniqueCandidate = true;
        for (int i=row; i<row+3; i++)           // Check if candidates exist in other elements
            for (int n=col; n<col+3; n++)
            {
                if (i==x && n==y) continue;     // Don't check self...
                if (g->cell[i][n][j] != false)  // If candidate exists in other cells
                    uniqueCandidate = false;    // Mark as not unique solution
            }

        if (uniqueCandidate == true)            // If solution is unique...
        {
            for (int d=1; d<10; d++)
                if (d != j)
                    g->cell[x][y][d] = 0;       // Zero out other candidates from x, y
            return;
        }
    }
}

void crossCheckLineCandidates(puzzle *g, int x, int y) {
    for (int j=1; j<10; j++)                     // For each candidate for x, y
    {
        if (g->cell[x][y][j] == false) continue; // Skip non-candidates

        bool uniqueCandidate = true;
        for (int i=0; i<9; i++)                  // Check if candidates exist in row elements
        {
            if (i == x) continue;                // But don't check self...
            if (g->cell[i][y][j] != 0)           // If candidate is not unique
                uniqueCandidate = false;         // Mark as not unique
        }

        if (uniqueCandidate == true)             // If solution is unique...
        {
            for (int d=0; d<9; d++)
                if (d != j)
                    g->cell[x][y][d] = false;    // Zero out other candidates from x, y
            return;
        }

        uniqueCandidate = true;
        for (int i=0; i<9; i++)                  // Check if candidates exist in col elements
        {
            if (i == y) continue;                // Don't check self...
            if (g->cell[x][i][j] != 0)           // If candidate is not unique
                uniqueCandidate = false;         // Mark as not unique
        }

        if (uniqueCandidate == true)             // If solution is unique...
        {
            for (int d=1; d<10; d++)
                if (d != j)
                    g->cell[x][y][d] = false;    // Zero out other candidates from x, y
            return;
        }
    }
}

void solveGrid(puzzle *g) {
    bool changed = true;
    while (changed == true)
    {
        changed = false;
        for (int x=0; x<9; x++)                                   // Get candidates & easy solutions
            for (int y=0; y<9; y++)
            {
                if (g->cell[x][y][0] > 0) continue;               // Skip non-empty cells
                for (int j=1; j<10; j++) g->cell[x][y][j] = true; // Populate candidate values
                eliminateNumberInLine(g, x, y);                   // Eliminate candidates from row & col
                eliminateNumberInBox(g, x, y);                    // Eliminate candidates from box
                int newValue = isSingleCandidate(g, x, y);        // See if single candidate remains
                if (newValue != 0)
                {
                    g->cell[x][y][0] = newValue;                  // Apply result to cell
                    changed = true;                               // Keep track that change was made
                }
        }
        if (changed == true) continue;

        for (int x=0; x<9; x++)                                   // Use candidates to find harder solutions
            for (int y=0; y<9; y++)
            {
                if (g->cell[x][y][0] > 0) continue;               // Skip non-empty cells
                crossCheckLineCandidates(g, x, y);                // Check for elements that can only work in current cell
                crossCheckBoxCandidates(g, x, y);                 // Check for elements that can only work in current cell
                int newValue = isSingleCandidate(g, x, y);        // See if single candidate remains
                if (newValue != 0)
                {
                    g->cell[x][y][0] = newValue;                  // Apply result to cell
                    changed = true;                               // Keep track that change was made
                }
            }
    }
}

void copyPuzzle(puzzle g, puzzle *t) {
    for (int x=0; x<9; x++)
        for (int y=0; y<9; y++)
            for (int j=0; j<10; j++)
                t->cell[x][y][j] = g.cell[x][y][j];
}

void guessSolution(puzzle *g, int level) {
    if (level > 2) return;                              // Don't allow recursion beyond 2 to happen.
    if (isSolved(*g)) return;                           // Don't recurse if solved

    for (int x=0; x<9; x++)
        for (int y=0; y<9; y++)
            if (g->cell[x][y][0] == 0)                  // Look for cells that aren't solved.
            {
                bool movePossible = false;              // Keep track of possible moves
                for (int j=1; j<10; j++)
                {
                    puzzle t;
                    copyPuzzle(*g, &t);                 // Copy puzzle so we keep original intact
                    if (t.cell[x][y][j] == true)        // See if j is a candidate number
                    {
                        movePossible = true;            // We have a valid move to try
                        t.cell[x][y][0] = j;            // Set cell to candidate value
                        solveGrid(&t);                  // Try solving grid
                        if (isSolved(t))                // If solved,
                        {
                            copyPuzzle(t, g);           // Copy solved puzzle back to master
                            return;                     // and return!
                        }
                        else
                        {
                            guessSolution(&t, level+1); // Recurse back if not yet solved
                            if (isSolved(t))            // If that did the trick
                            {
                                copyPuzzle(t, g);       // Copy back to master
                                return;                 // And return!
                            }
                        }
                    }
                }
                if (movePossible == false)              // If no valid moves are found
                    return;                             // Dead end. Return...
            }
}

bool isValidInput(char str[9]) {
    // Check for length of string
    if (strlen(str) != 9)
        return false;

    // Make sure all characters are digits
    for (int i=0; i<9; i++)
        if (str[i] < '0' || str[i] > '9')
            return false;

    return true;
}

int main() {
    // Struct for puzzle
    puzzle sudoku = {};

    if (isatty(0))
        // Read in game grid
        //     0        10        20        30        40        50        60        70        80
        //     |---------|---------|---------|---------|---------|---------|---------|---------|
        printf("Please input each row of the unsolved puzzle without spaces. Enter only 9 \n" \
               "digits using 0 to denote unknown values (eg. 100305400).\n\n");

    for (int row = 0; row<9; row++)
    {
        char input[9] = "";

        // Verify that puzzle is valid as input so far
        bool validPuzzle = false;
        do {

            // Verify that current line input is valid
            bool validInput = false;
            do {

                // Get input from the user
                if (isatty(0))
                    printf("Row %d: ", row+1);
                scanf("%s", input);

                // Verify 9 digits
                validInput = isValidInput(input);
                if (validInput == false)
                    printf("Invalid input on row %d! Must be 9 digits, and not \"%s\"\n", row+1, input);
            } while (validInput == false && isatty(0));

            // Copy input digits to structure
            for (int col=0; col<9; col++)
                sudoku.cell[row][col][0] = input[col]-'0';

            // Verify that new row contains valid puzzle
            validPuzzle = isValidPuzzle(sudoku);
            if (validPuzzle == false)
                printf("Line invalidates puzzle. Please re-enter line.\n");
            if (!isatty(0) && (!validPuzzle || !validInput))
                exit(1);
        } while (validPuzzle == false && isatty(0));
    }

    // Print input puzzle
    printPuzzle(sudoku);

    // Simple deduction first
    solveGrid(&sudoku);

    // Make some guesses to solve
    if (isSolved(sudoku) == false)
        guessSolution(&sudoku, 1);

    // Print solution
    printPuzzle(sudoku);

    //if (isSolved(sudoku) == false)
    //printf("Unable to solve.");

    return 0;
}
