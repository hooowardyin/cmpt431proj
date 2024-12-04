#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <thread>
#include "get_time.h"
#include <fstream>
#include <sstream>
#include <string>

// #define sqr(x) ((x) * (x))
//#define DEFAULT_S "CMPT431-2024FALL"
#define DEFAULT_NUMBER_OF_THREADS "1"
#define DEFAULT_INPUT_FILE "input.txt"

class DPTable
{

private:

    std::vector< std::vector <int> >* table;


public:
    uint sLength;
    std::string inputS;

    DPTable(std::string s) {    // create the DPTable based on the input string
        inputS = s;
        sLength = inputS.size() - 1;
        table = new std::vector< std::vector <int> >(sLength, std::vector<int>(sLength, 0));


    };
    ~DPTable() {
        delete table;
    };

    void assign(uint x, uint y, uint newV) {
        (*table)[y][x] = newV;
    };

    uint read(uint x, uint y) {
        return (*table)[y][x];
    };

    void printTable() {
        for (int y = 0; y < sLength; y++) {
            for (int x = 0; x < sLength; x++) {
                // std::cout << "( " << x << ", " << y << " ) = " << read(x, y);
                std::cout << read(x, y) << ", ";
                if (x == sLength - 1) {
                    std::cout << "\n";
                }
            }
        }
    }

};


void fillTableThread(DPTable* T, uint startCol, uint endCol, int threadCount, int threadId, std::vector<double>& time_each_threads) {
    // Timer to measure the time taken by each thread
    timer threadTimer;
    threadTimer.start();
    double threadTimeTaken;

    // Iterate through the rows of the DP table (bottom to top)
    for (int y = T->sLength - 1; y >= 0; y--) {
        // Iterate through the columns assigned to this thread
        for (int x = startCol; x <= endCol; x++) {
            if (y > x) {
                // Skip cells above the diagonal (not needed for palindrome subsequences)
                continue;
            }

            if (x == y) {
                // Diagonal cells (base case): A single character is a palindrome of length 1
                T->assign(x, y, 1);
            }
            else if (x == y + 1) {
                // Cells directly above the diagonal: Compare two adjacent characters
                if (T->inputS[x] == T->inputS[y]) {
                    T->assign(x, y, 2); // Characters match, palindrome length is 2
                }
                else {
                    T->assign(x, y, 1); // Characters do not match, length is 1
                }
            }
            else {
                // For other cells, calculate the value based on dependencies:
                // Wait for the left and bottom-left dependencies to be ready
                while (x > startCol && T->read(x - 1, y) == 0) {
                    std::this_thread::yield(); // Yield to allow other threads to make progress
                }
                while (y + 1 < T->sLength && T->read(x, y + 1) == 0) {
                    std::this_thread::yield();
                }

                // Retrieve the values of dependencies
                int left = T->read(x - 1, y);
                int bot = T->read(x, y + 1);
                if (T->inputS[x] == T->inputS[y]) {
                    // Characters match, add 2 to the value of the bottom-left dependency
                    int leftBot = T->read(x - 1, y + 1);
                    T->assign(x, y, leftBot + 2);
                }
                else {
                    // Characters do not match, take the maximum of left or bottom dependency
                    T->assign(x, y, std::max(left, bot));
                }
            }
        }
    }
    // Stop the timer for this thread and record the time taken
    threadTimeTaken = threadTimer.stop();
    time_each_threads[threadId] = threadTimeTaken;
}

uint longestPalindromeSubseq(DPTable* T, uint nThreads) {
    // Timer to measure the total execution time for all threads
    timer parallelTimer;
    parallelTimer.start();
    double timeTaken;

    // Create threads for parallel computation
    std::vector<std::thread> threads;
    uint sLength = T->sLength;

    // Divide the DP table columns among threads
    uint subCols = sLength / nThreads;       // Base number of columns per thread
    uint extraCols = sLength % nThreads;    // Extra columns to distribute among the first few threads

    // Store the start and end columns for each thread
    std::vector<uint> threadStartCol(nThreads);
    std::vector<uint> threadEndCol(nThreads);

    // Vector to record the time taken by each thread
    std::vector<double> time_each_threads(nThreads, 0.0);

    // Create and launch threads
    for (uint i = 0; i < nThreads; i++) {
        // Calculate the start and end columns for this thread
        uint startCol = i * subCols + std::min(i, extraCols);
        uint endCol = startCol + subCols - 1 + (i < extraCols ? 1 : 0);
        threadStartCol[i] = startCol;
        threadEndCol[i] = endCol;

        // Launch the thread with its assigned range of columns
        threads.emplace_back(fillTableThread, T, startCol, endCol, nThreads, i, std::ref(time_each_threads));
    }

    // Wait for all threads to finish
    for (auto& th : threads) {
        th.join();
    }

    // Print timing information for each thread
    std::cout << "threadId, timeTaken, startCol, endCol" << std::endl;
    for (uint i = 0; i < nThreads; i++) {
        std::cout << i << ", " << time_each_threads[i] << ", " << threadStartCol[i] << ", " << threadEndCol[i] << "\n";
    }

    // Stop the total timer and print the total execution time
    timeTaken = parallelTimer.stop();
    std::cout << "Total time using (in seconds): " << timeTaken << "\n";

    // Return the result from the top-right cell of the DP table
    return T->read((T->sLength) - 1, 0);
}



// args: inputS, nThreads,
//  -- inputFile: the original input file to check longest sub sequence palindrome. Default: "input.txt"
//  -- nThreads: number of forked process. default value: 1


int main(int argc, char* argv[]) {
    //std::string inputS;
    std::ifstream inputFile;
    uint nThreads;

    if ((argc > 3) || (argc == 2)) {
        std::cout << "Number of args is not correct for the program (1. inputFile, 2. nThreads)\n";
        exit(1);
    }

    if (argc == 1) {
        inputFile.open(DEFAULT_INPUT_FILE);
        nThreads = std::stoi(DEFAULT_NUMBER_OF_THREADS);
    }
    else {
        inputFile.open(argv[1]);
        nThreads = std::stoi(argv[2]);
    }

    std::string fileContents((std::istreambuf_iterator<char>(inputFile)),
        std::istreambuf_iterator<char>());

    DPTable* DBT = new DPTable(fileContents);

    //std::cout << "The input string is: " << fileContents << "\n";
    std::cout << "The number of threads is: " << nThreads << "\n";

    uint length = longestPalindromeSubseq(DBT, nThreads);
    delete DBT;

    std::cout << "The longest palindrome subsequence has a length of: " << length << "\n";

    return 0;
}