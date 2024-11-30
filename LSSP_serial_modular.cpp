#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include "get_time.h"

// #define sqr(x) ((x) * (x))
#define DEFAULT_S "CMPT431-2024FALL"
#define DEFAULT_NUMBER_OF_PROCESS "1"

class DPTable
{

private:

  std::vector< std::vector <int> >* table;


public:
  uint sLength;
  std::string inputS;

  DPTable(std::string s){    // create the DPTable based on the input string
    inputS = s;
    sLength = inputS.size();
    table = new std::vector< std::vector <int> > (sLength, std::vector<int> (sLength, 0));


  };
  ~DPTable(){
    delete table;
  };

  void assign(uint x, uint y, uint newV){
    (*table)[y][x] = newV;
  };

  uint read(uint x, uint y){
    return (*table)[y][x];
  };

  void printTable(){
    for (int y = 0; y < sLength; y++){
      for(int x = 0; x <sLength; x++){
        std::cout << "( " << x << ", " << y << " ) = " << read(x, y) << "\n";
      }
    }
  }
  
};


void fillTable(DPTable *T){
  for(int y = T -> sLength - 1; y >= 0 ; y--){
    for (int x = 0 ; x <  T -> sLength; x++){
      if(y > x){
        continue;
      }
      else{
        if(x == y){
          T->assign(x, y, 1);
        }
        else{
          if ( T -> inputS[x-1] ==  T -> inputS[y-1]){
            // (*T->table)[y][x] =  (*T->table)[y+1][x-1] + 2;
            T->assign(x, y, T->read(x-1, y+1) + 2);
          }
          else{
            // int left = (*T->table)[y][x-1];
            // int bot = (*T->table)[y+1][x];

            int left = T->read(x-1, y);
            int bot = T->read(x, y+1);
            int temp = left > bot ? left : bot;
             // (*T->table)[y][x] = temp;
              T->assign(x, y, temp);
          }
        }
      }
    }
  }
}

uint longestPalindromeSubseq(DPTable *T, uint nProcess) {

  timer serialTimer;
  serialTimer.start();
  double timeTaken;

  // ----------- serial OR multi_thread OR MPI ----------------- //
  fillTable(T);



  timeTaken = serialTimer.stop();
  std::cout << "total time using(in seconds): " 
            << timeTaken << "\n";

  // return (*T->table)[0][T->sLength-1];
  return T->read((T->sLength) - 1, 0);
}


// args: inputS, nProcess,
//  -- inputS: the original input string to check longest sub sequence parlindrom. default value: "CMPT431-2024FALL"
//  -- nProcess: number of forked process. default value: 1


int main(int argc, char *argv[]) {

  // Initialize command line arguments

  std::string inputS;
  uint nProcess;

  if(argc > 3){
    std::cout << "number of args is not correct for the program (1. inputS, 2. nProcess)\n";
    exit(1);
  }

  if(argc == 1){
    inputS = DEFAULT_S;
    nProcess = std::stoi(DEFAULT_NUMBER_OF_PROCESS);
  }

  else if(argc == 2){
    inputS = argv[1];    
    nProcess = std::stoi(DEFAULT_NUMBER_OF_PROCESS);
  }

  else{
    inputS = argv[1];
    nProcess = std::stoi(argv[2]);
  }

  DPTable *DBT = new DPTable(inputS);

  std::cout << "the input string is: " << inputS << "\n";;
  std::cout << "the number of process is: " << nProcess << "\n";

//  uint length = longestPalindromeSubseq(inputS, nProcess);
  uint length = longestPalindromeSubseq(DBT, nProcess);
  // DBT->printTable();
  delete DBT;

  std::cout << "the longest palindrome subseq has a length of: " << length << "\n";

  return 0;
}









