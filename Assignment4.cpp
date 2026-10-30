/*
 * Program name: EECS 348 Assignment 4 - Sudoku Solver
 * Description: Object-oriented C++ program that solves Sudoku puzzles with a
 *              recursive depth-first search with backtracking. It prints
 *              every solution for each puzzle, or "No solution found".
 * Inputs: puzzle1.txt through puzzle5.txt (9x9 grids, "_" marks a blank cell)
 * Outputs: For each file, the file name, the original puzzle, and every
 *          solution, or "No solution found"
 * Author: Cesar Olono
 * Other sources: Claude wrote the base program (the original
 *                classes and the recursive search).
 * Creation date: 10/01/2026
 * Revision date: 10/07/2026
 */

#include <fstream> //lets the program read the puzzle files
#include <iostream> //lets the program print to the screen
#include <string> //string for file names
#include <vector> //vectore for the grid that stores the board

// ---------------------------------------------------------------------------
// SudokuBoard: owns the 9x9 grid and knows the Sudoku rules.
// A cell value of 0 means "empty" internally; '_' is used for I/O.
// ---------------------------------------------------------------------------

class SudokuBoard {  //class SudokuBoard is one objext that holds the 9x9 puzzle and understands how to work with it
public:
    static const int SIZE = 9;    //SIZE and BLOCK are the numbers for the grid size
    static const int BLOCK = 3;   //they're useful for not having to type 9 and 3 everywhere

    SudokuBoard() : grid_(SIZE, std::vector<int>(SIZE, 0)) {}

    // Load a puzzle from a file. Returns false if the file can't be opened, has characters other than digits,
    // _ and whitespace, or doesn't have exactly 81 cells.
        bool loadFromFile(const std::string& filename) { //reads a puzzle file into the grid one at a time,
        std::ifstream in(filename.c_str());             //and returns false if the file is bad or doesn't have 81 cells
        if (!in) {
            return false;
        }

        int count = 0;
        char ch;
        while (in.get(ch)) {
            if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') {
                continue;                // whitespace between cells is fine
            }
            if (count >= SIZE * SIZE) {
                return false;            // more than 81 cells in the file
            }
            if (ch == '_') {
                grid_[count / SIZE][count % SIZE] = 0;
            } else if (ch >= '1' && ch <= '9') {
                grid_[count / SIZE][count % SIZE] = ch - '0';
            } else {
                return false;            // a character that isn't a cell
            }
            ++count;
        }
        return count == SIZE * SIZE;
    }

    void set(int row, int col, int value) { grid_[row][col] = value; } //puts a number into one cell of the grid


    // Is 'digit' allowed at (row, col)? Checks row, column and 3x3 block.
    bool isValidPlacement(int row, int col, int digit) const { //checks that a digit isn't already in that cell's row, column, or block
        for (int i = 0; i < SIZE; ++i) {
            if (grid_[row][i] == digit) return false;  // same row
            if (grid_[i][col] == digit) return false;  // same column
        }
        int blockRow = (row / BLOCK) * BLOCK;
        int blockCol = (col / BLOCK) * BLOCK;
        for (int r = blockRow; r < blockRow + BLOCK; ++r) {
            for (int c = blockCol; c < blockCol + BLOCK; ++c) {
                if (grid_[r][c] == digit) return false;  // same block
            }
        }
        return true;
    }

    // Count how many digits (1-9) could legally go in this cell.
    int countCandidates(int row, int col) const { //counts how many digits from 1 to 9 could legally go in an empty cell
        int count = 0;
        for (int digit = 1; digit <= 9; ++digit) {
            if (isValidPlacement(row, col, digit)) {
                ++count;
            }
        }
        return count;
    }
    // Find the empty cell with the fewest valid digits. Returns false if
    // there are no empty cells.
    bool findBestEmpty(int& row, int& col) const { //finds the empty cell with the least digis so the solver guesses athe the most force cell
        int bestCount = 10;  // higher than any real count (max is 9)
        bool found = false;
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                if (grid_[r][c] == 0) {
                    int count = countCandidates(r, c);
                    if (count < bestCount) {
                        bestCount = count;
                        row = r;
                        col = c;
                        found = true;
                    }
                }
            }
        }
        return found;
    }
    // Do the pre-filled cells already break a rule? (e.g. two 5s in a row)
    bool givensAreConsistent() { //checks that the digits already in the puzzle don't break a rule before solving starts
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                int digit = grid_[r][c];
                if (digit == 0) continue;
                grid_[r][c] = 0;  // temporarily remove to test against the rest
                bool ok = isValidPlacement(r, c, digit);
                grid_[r][c] = digit;
                if (!ok) return false;
            }
        }
        return true;
    }

    void print(std::ostream& out) const { //prints the grid with _ for empty cells
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                if (c > 0) out << ' ';
                if (grid_[r][c] == 0) out << '_';
                else out << grid_[r][c];
            }
            out << '\n';
        }
    }

private:
    std::vector<std::vector<int> > grid_;
};

// ---------------------------------------------------------------------------
// SudokuSolver: recursive DFS + backtracking. Prints every solution as it is found.
// ---------------------------------------------------------------------------
class SudokuSolver { //does the recursive search trying digits and backs up when stuck and prints every solution as it finds them
public:
    SudokuSolver() : solutionCount_(0) {}

    // Solve a copy of the given puzzle. Each solution is printed as soon
    // as it is found. Returns how many solutions were found.
    int solveAll(const SudokuBoard& puzzle) {
        solutionCount_ = 0;
        SudokuBoard working = puzzle;
        if (working.givensAreConsistent()) {
            solve(working);
        }
        return solutionCount_;
    }

private:
    int solutionCount_;

    void solve(SudokuBoard& board) {
        int row, col;
        if (!board.findBestEmpty(row, col)) {
            ++solutionCount_;  // no empty cells: a full solution
            std::cout << "Solution " << solutionCount_ << ":\n";
            board.print(std::cout);
            std::cout << "\n";
            return;
        }

        for (int digit = 1; digit <= 9; ++digit) {
            if (board.isValidPlacement(row, col, digit)) {
                board.set(row, col, digit);  // try it
                solve(board);                // recurse
                board.set(row, col, 0);      // undo (backtrack)
            }
        }
        // Keep going after a success so that *all* solutions are found.
    }
};

// ---------------------------------------------------------------------------
// PuzzleRunner: handles files and output formatting for each puzzle.
// ---------------------------------------------------------------------------
class PuzzleRunner { //loads one puzzle file, prints it, runs the solver and prints "No solution found" if nothing worked
public:
    void run(const std::string& filename) {
        std::cout << filename << "\n";

        SudokuBoard puzzle;
        if (!puzzle.loadFromFile(filename)) {
            std::cout << "Error: could not read a valid puzzle from " << filename << "\n\n";
            return;
        }

        std::cout << "Original puzzle:\n";
        puzzle.print(std::cout);
        std::cout << "\n";

        SudokuSolver solver;
        int found = solver.solveAll(puzzle);

        if (found == 0) {
            std::cout << "No solution found\n";
        }
        std::cout << "\n";
    }
};

int main() { //runs the runner on puzzle 1 through 5
    PuzzleRunner runner;
    for (int i = 1; i <= 5; ++i) {
        runner.run("puzzle" + std::to_string(i) + ".txt");
    }
    return 0;
}
