#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include "get_time.h"
#include <mpi.h>


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
        // std::cout << "( " << x << ", " << y << " ) = " << read(x, y) << "\n";
        std::cout << read(x, y) << ", ";
        if(x == sLength - 1){
          std::cout << "\n";
        }
      }
    }
  }
  
};


void fillTableMPI(DPTable *T, uint startCol, uint endCol, int worldSize, int worldRank){



  for(int y = T->sLength - 1; y >= 0 ; y--){
    for (int x = startCol ; x <= endCol; x++){
      if(y > x){
        continue;
      }


      else{

        // first non-zero diagonals
        if(x == y){
          T->assign(x, y, 1);
        }

        // second non-zero diagonals
        else if( x == y + 1){
          int toBeSent;
          if ( T -> inputS[x] ==  T -> inputS[y]){
            T->assign(x, y, 2);
            toBeSent = 2;
          }
          else{
            T->assign(x, y, 1);
            toBeSent = 1;
          }

          // if it is the edge column && not the last col

          //  send to right
          if(x == endCol && worldRank < worldSize - 1){
            MPI_Send(              
                /* data         = */ &toBeSent, 
                /* count        = */ 1,
                /* datatype     = */ MPI_INT, 
                /* destination  = */ worldRank + 1, 
                /* tag          = */ y,             // same level 
                /* communicator = */ MPI_COMM_WORLD);
          }

          
        }
        // other diagonals
        else{
          int leftBot;
          int left;
          int toBeSent;

          // except the first col, recv first
          // if(x+2 == y){
          //   T->assign(x-1, y+1, 1);
          // }

          if(x == startCol && worldRank>0 && y < T->sLength - 2){
            MPI_Recv(
                /* data         = */ &left, 
                /* count        = */ 1,
                /* datatype     = */ MPI_INT,
                /* source       = */ worldRank - 1, 
                /* tag          = */ y,       // same level 
                /* communicator = */ MPI_COMM_WORLD, 
                /* status       = */ MPI_STATUS_IGNORE);

            //fill the recieved value to table //
            T->assign(x - 1, y, left);
          }

          

          // for internal col, read from table directly
          // if(x > startCol){
          //   left = T->read(x - 1, y);
          //   leftBot = T->read(x - 1, y + 1);
          // }

          // ---------- scenario 1, find a match char ----------------//
          if ( T -> inputS[x] ==  T -> inputS[y]){
            if(x == startCol && x == y+2){
              T->assign(x-1, y+1, 1);
            }

            leftBot = T->read(x - 1, y + 1);
            T->assign(x, y, leftBot + 2);
            toBeSent = leftBot + 2;
    
          }
          // ----------- scenario 2, not match ---------------------//
          else{
            left = T->read(x - 1, y);
            int bot = T->read(x, y+1);
            int temp = left > bot ? left : bot;
             // (*T->table)[y][x] = temp;
              T->assign(x, y, temp);
              toBeSent = temp;
          }

          // ------------- send to next -----------------//
          // except the last process && the edge column && 
          //  send to up-right
          if( x == endCol && worldRank < worldSize - 1){
            MPI_Send(              
                /* data         = */ &toBeSent, 
                /* count        = */ 1, 
                /* datatype     = */ MPI_INT, 
                /* destination  = */ worldRank + 1, 
                /* tag          = */ y,             // top level 
                /* communicator = */ MPI_COMM_WORLD);
          }





        }
      }
    }
  }
}

void longestPalindromeSubseq(DPTable *T, uint nProcess) {

  timer serialTimer;
  serialTimer.start();
  double timeTaken;


  // initialize MPI

  MPI_Init(NULL, NULL);
  int worldSize;
  MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

  int worldRank;
  MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);

  // ------------- split the work to each process ---------------//
  uint startCol;
  uint endCol;

  uint subCols = T->sLength / worldSize;
  uint extraCols = T->sLength % worldSize;

  if(worldRank < extraCols){  // --------- assign extra column to first extraCols-th process -------//
    startCol = (subCols + 1) * worldRank;
    endCol = startCol + subCols;
  }
  else{
    startCol = (subCols + 1) * extraCols + (worldRank - extraCols) * subCols;
    endCol = startCol + subCols - 1;
  }


  // ----------- serial OR multi_thread OR MPI ----------------- //
  fillTableMPI(T, startCol, endCol, worldSize, worldRank);



  timeTaken = serialTimer.stop();
  

  // return (*T->table)[0][T->sLength-1];
    // for(int i = 0; i < worldSize; i++){
    //   if(worldRank == i){
    //     std::cout<<"portion from process: "<<worldRank<<"\n";
    //     T->printTable();
    //     std::cout<<"\n";
        
    //   }
    //   MPI_Barrier(MPI_COMM_WORLD);
    // }

  if(worldRank == worldSize - 1){
    // T->printTable();
    std::cout << "the final result is: " << T->read((T->sLength) - 1, 0) << "\n";
    std::cout << "total time using(in seconds): " 
            << timeTaken << "\n";
  }
  MPI_Finalize();
  // return T->read((T->sLength) - 1, 0);
  return;
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

  // std::cout << "the input string is: " << inputS << "\n";;
  // std::cout << "the number of process is: " << nProcess << "\n";

//  uint length = longestPalindromeSubseq(inputS, nProcess);
  longestPalindromeSubseq(DBT, nProcess);
  // DBT->printTable();
  delete DBT;

  // std::cout << "the longest palindrome subseq has a length of: " << length << "\n";

  return 0;
}









