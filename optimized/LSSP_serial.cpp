#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include "get_time.h"
#include <fstream>
#include <sstream>
#include <string>

// #define sqr(x) ((x) * (x))
#define DEFAULT_S "CMPT431-2024FALL"
#define DEFAULT_NUMBER_OF_PROCESS "1"
#define DEFAULT_INPUT_FILE "input.txt"



class DPTable
{

private:

  std::vector< std::vector <int> >* table;


public:
  uint sLength;
  std::string inputS;

  DPTable(std::string s){    // create the DPTable based on the input string
    inputS = s;
    sLength = inputS.size()-1;
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
        // std::cout << "( " << x << ", " << y << " ) = " << read(x, y);
        std::cout << read(x,y) << ", ";
        if (x == sLength - 1){
          std::cout<<"\n";
        }
      }
    }
  }
  
};


void fillTable(DPTable *T){
  for(int y = T -> sLength - 1; y >= 0 ; y--){
    T->assign(y, y, 1);
    for (int x = y + 1 ; x <  T -> sLength; x++){


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

uint longestPalindromeSubseq(DPTable *T) {

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
  std::ifstream inputFile;

  if(argc > 2){
    std::cout << "number of args is not correct for the program (1. inputS)\n";
    exit(1);
  }

  if(argc == 1){
    inputFile.open(DEFAULT_INPUT_FILE);
  }
  else{
    inputFile.open(argv[1]);
  }
  
  std::string fileContents((std::istreambuf_iterator<char>(inputFile)),
                             std::istreambuf_iterator<char>());


  inputFile.close();
  DPTable *DBT = new DPTable(fileContents)  ;
  // std::cout << " input string have a length of " << DBT -> sLength <<"\n";


  int length = longestPalindromeSubseq(DBT);
  // DBT->printTable();
  delete DBT;



  std::cout << "the longest palindrome subseq has a length of: " << length << "\n";

  return 0;
}









