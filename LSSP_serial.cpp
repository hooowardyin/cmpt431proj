#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include "get_time.h"

// #define sqr(x) ((x) * (x))
#define DEFAULT_S "CMPT431-2024FALL"
#define DEFAULT_NUMBER_OF_PROCESS "1"



uint longestPalindromeSubseq(std::string s, uint nProcess) {

  timer serialTimer;
  serialTimer.start();
  double timeTaken;

  int sLength = s.size();

  std::vector< std::vector <int> > table (sLength + 1, std::vector<int> (sLength + 1, 0));

  for(int y = sLength; y >= 1 ; y--){
      for (int x = 1 ; x <= sLength; x++){
          if(y > x){
              continue;
          }
          else{
              if(x == y){
                  table[y][x] = 1;
              }
              else{
                  if (s[x-1] == s[y-1]){
                      table[y][x] = table[y+1][x-1] + 2;
                  }
                  else{
                      int left = table[y][x-1];
                      int bot = table[y+1][x];
                      int temp = left > bot ? left : bot;
                      table[y][x] = temp;
                  }

                  // cout<<" (table " << y <<", "<< x << ") : "<< table[y][x]<<endl;
              }
          }
      }
  }

  timeTaken = serialTimer.stop();
  std::cout << "total time using(in seconds): " 
            << timeTaken << "\n";

  return table[1][sLength];
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



  std::cout << "the input string is: " << inputS << "\n";;
  std::cout << "the number of process is: " << nProcess << "\n";

  uint length = longestPalindromeSubseq(inputS, nProcess);
  std::cout << "the longest palindrome subseq has a length of: " << length << "\n";

  return 0;
}









