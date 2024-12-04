#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include "get_time.h"
#include <fstream>
#include <sstream>
#include <string>


#define DEFAULT_INPUT_FILE "input/big.txt"



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
    for (int x = 0 ; x <  T -> sLength; x++){
	
      if(y > x){
        continue;
      }
      else{ // fill the first non-zero diagonals
      	 if(x == y){
           T->assign(x, y, 1);
        }
        else{
          // --------- senario 1, find equal char, fill (left bot + 2) to current cell
          if ( T -> inputS[x] ==  T -> inputS[y]){

            T->assign(x, y, T->read(x-1, y+1) + 2);
          }
          // ---------- senario 2, not equal, fill the max of left and bot to current cell
          else{

            int left = T->read(x-1, y);
            int bot = T->read(x, y+1);
            int temp = left > bot ? left : bot;

            T->assign(x, y, temp);
          }
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

  return T->read((T->sLength) - 1, 0);
}


// args: inputS, nProcess,
//  -- inputS: file path, the file stores the input string to check longest sub sequence parlindrom.
//  


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
  
  // -------- load the input string from the file ------------//
  std::string fileContents((std::istreambuf_iterator<char>(inputFile)),
                             std::istreambuf_iterator<char>());


  inputFile.close();

  // ----------------- build the DPtable ------------------//
  DPTable *DBT = new DPTable(fileContents)  ;


  int length = longestPalindromeSubseq(DBT);
  // DBT->printTable();
  delete DBT;



  std::cout << "the longest palindrome subseq has a length of: " << length << "\n";

  return 0;
}









