#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include "get_time.h"
#include <mpi.h>
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
        // std::cout << "( " << x << ", " << y << " ) = " << read(x, y) << "\n";
        std::cout << read(x, y) << ", ";
        if(x == sLength - 1){
          std::cout << "\n";
        }
      }
    }
  }

  // void buildMPImsg (std::vector<int> *edgeData, int startRow, int endRow, int colNumber){
  void buildMPImsg (int *edgeData, int startRow, int endRow, int colNumber){

    //define the acctual size of the edge
    int subJobs = endRow - startRow + 1;
    edgeData[0] = subJobs;
    // std::cout<< "before send the length of the edge data is: "<<edgeData[0]<<"\n";

    for(int i = 1; i < subJobs + 1 ; i++){
      edgeData[i] = read(colNumber, startRow + i - 1);
      // std::cout<< "the "<<i<<"th edgeData item is: "<<edgeData[i]<<"\n";
    }
    return;
  }

  int MPImsgToTable (int *edgeData, int startRow, int endRow, int colNumber){
    // this function will fill the table AND return the size of the edge
    int subJobs = edgeData[0];
    // std::cout<< "the length of the edge data to be filled has a length of : "<<edgeData[0]<<"\n";
    for(int i = 0; i < subJobs ; i++){
      assign(colNumber, startRow + i, edgeData[i+1]);
      // std::cout<< "the "<<i<<"th to be saved edgeData item is: "<<edgeData[i]<<"\n";
    }
    return subJobs;
  }

  
};


void fillTableMPI(DPTable *T, uint startCol, uint endCol, int worldSize, int worldRank){



  for(int y = endCol; y >= 0 ; y--){
    T->assign(y, y, 1);
    for (int x = y + 1 ; x <= endCol; x++){
      // if(y > x){
      //   continue;
      // }


      // else{

        // first non-zero diagonals
        // if(x == y){
        //   T->assign(x, y, 1);
        // }

        // second non-zero diagonals
        if( x == y + 1){
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
      // }
    }
  }
}

void fillTableMPIOptimization(DPTable *T, uint startCol, uint endCol, int worldSize, int worldRank, int* edgeData){

  // read the current job size
  int subJobs = endCol - startCol + 1;
  int jobCount = 0;
  int commTurn = 0;
  // -------------- start from the diagonals ------------------------//
  for(int y = endCol; y >= 0 ; y--){
    T->assign(y, y, 1);
    for (int x = y + 1 ; x <= endCol; x++){

      int leftBot;
      int left;


      // ---------- scenario 1, find a match char ----------------//
      if ( T -> inputS[x] ==  T -> inputS[y]){

        leftBot = T->read(x - 1, y + 1);
        T->assign(x, y, leftBot + 2);

      }
      // ----------- scenario 2, not match ---------------------//
      else{
        left = T->read(x - 1, y);
        int bot = T->read(x, y+1);
        int temp = left > bot ? left : bot;
        T->assign(x, y, temp);
      }

    }


    // increment the jobCount
    jobCount++;

  // when process finished a non-dependent trangle or square //
  /*
        -----------------------------------------
        | \  |    |    |    |    |    |    |    |
        |  \ |    |    |    |    |    |    |    |
        -----------------------------------------
        |    | \  |    |    |    |    |    |    | 
        |    |  \ |    |    |    |    |    |    | 
        -----------------------------------------
        |    |    | \  |    |    |    |    |    |
        |    |    |  \ |    |    |    |    |    | 
        -----------------------------------------
        |    |    |    | \##|    |    |    |    |    <-- when the hashed area done,
        |    |    |    |  \#|    |    |    |    |         send the edge to pi+1
        -----------------------------------------
        |    |    |    |    | \  |    |    |    | 
        |    |    |    |    |  \ |    |    |    |
        -----------------------------------------
        |    |    |    |    |    | \  |    |    |   
        |    |    |    |    |    |  \ |    |    |  
        -----------------------------------------
        |    |    |    |    |    |    | \  |    |  
        |    |    |    |    |    |    |  \ |    |  
        -----------------------------------------
        |    |    |pi-1| pi |pi+1|    |    | \  |     
        |    |    |    |    |    |    |    |  \ |    
        -----------------------------------------
  
  */

    // ------------- send to next -----------------//

    if(jobCount == subJobs){

    // steps,  in even commTurn(communication turn), even ranked process send first
    //         in odd commTurn, even ranked process recv first

      // even turn: even rannked process send first, odd ranked ones recv first
      if( commTurn % 2 == 0){
        
        // even ranked p
        if(worldRank % 2 == 0){

          // the commTurn-th p not recv, the last p not send
          if(worldRank < worldSize -1){
            T->buildMPImsg(edgeData, y, y + subJobs - 1, endCol);



            MPI_Send(              
              /* data         = */ edgeData, 
              /* count        = */ sizeof(edgeData)/sizeof(int), 
              /* datatype     = */ MPI_INT, 
              /* destination  = */ worldRank + 1, 
              /* tag          = */ worldRank + 1,             // recv side
              /* communicator = */ MPI_COMM_WORLD);
          }

          if(worldRank > commTurn){
            MPI_Recv(
                /* data         = */ edgeData, 
                /* count        = */ sizeof(edgeData)/sizeof(int),
                /* datatype     = */ MPI_INT,
                /* source       = */ worldRank - 1, 
                /* tag          = */ worldRank,       // recv side
                /* communicator = */ MPI_COMM_WORLD, 
                /* status       = */ MPI_STATUS_IGNORE);

          // fill the table AND update the new subJobs, jobCount
            subJobs = T->MPImsgToTable(edgeData, y - 1 - edgeData[0] + 1, y - 1, startCol - 1);


          }   
        }
        // odd ranked p
        else{
          int newJobLength = subJobs;
          // the commTurn-th p not recv, the last p not send
          if(worldRank > commTurn){


            MPI_Recv(
                /* data         = */ edgeData, 
                /* count        = */ sizeof(edgeData)/sizeof(int),
                /* datatype     = */ MPI_INT,
                /* source       = */ worldRank - 1, 
                /* tag          = */ worldRank,       // recv side
                /* communicator = */ MPI_COMM_WORLD, 
                /* status       = */ MPI_STATUS_IGNORE);

            // fill the table

            newJobLength = T->MPImsgToTable(edgeData, y - 1 - edgeData[0] + 1, y - 1, startCol - 1);
          }
          
          if(worldRank < worldSize -1){
            // build the MPI msg
            T->buildMPImsg(edgeData, y, y + subJobs - 1, endCol);
            // send
            MPI_Send(              
            /* data         = */ edgeData, 
            /* count        = */ sizeof(edgeData)/sizeof(int), 
            /* datatype     = */ MPI_INT, 
            /* destination  = */ worldRank + 1, 
            /* tag          = */ worldRank + 1,             // recv side
            /* communicator = */ MPI_COMM_WORLD);
          }
          subJobs = newJobLength;

        }

      }
      // odd turn: odd rannked process send first, even ranked ones recv first
      else{
                
        // odd ranked p
        if(worldRank % 2 == 1){

          // the commTurn-th p not recv, the last p not send
          if(worldRank < worldSize -1){
            T->buildMPImsg(edgeData, y, y + subJobs - 1, endCol);
            MPI_Send(              
              /* data         = */ edgeData, 
              /* count        = */ sizeof(edgeData)/sizeof(int), 
              /* datatype     = */ MPI_INT, 
              /* destination  = */ worldRank + 1, 
              /* tag          = */ worldRank + 1,             // recv side
              /* communicator = */ MPI_COMM_WORLD);
          }

          if(worldRank > commTurn){
            MPI_Recv(
                /* data         = */ edgeData, 
                /* count        = */ sizeof(edgeData)/sizeof(int),
                /* datatype     = */ MPI_INT,
                /* source       = */ worldRank - 1, 
                /* tag          = */ worldRank,       // recv side
                /* communicator = */ MPI_COMM_WORLD, 
                /* status       = */ MPI_STATUS_IGNORE);

          // fill the table AND update the new subJobs, jobCount
            subJobs = T->MPImsgToTable(edgeData, y - 1 - edgeData[0] + 1, y - 1, startCol - 1);
          }   
        }
        // even ranked p
        else{
          // the commTurn-th p not recv, the last p not send
          int newJobLength = subJobs;
          if(worldRank > commTurn){
            MPI_Recv(
                /* data         = */ edgeData, 
                /* count        = */ sizeof(edgeData)/sizeof(int),
                /* datatype     = */ MPI_INT,
                /* source       = */ worldRank - 1, 
                /* tag          = */ worldRank,       // recv side
                /* communicator = */ MPI_COMM_WORLD, 
                /* status       = */ MPI_STATUS_IGNORE);

            // fill the table
            newJobLength = T->MPImsgToTable(edgeData, y - 1 - edgeData[0] + 1, y - 1, startCol - 1);
          }
          
          if(worldRank < worldSize -1){
            // build the MPI msg
            T->buildMPImsg(edgeData, y, y + subJobs - 1, endCol);
            // send
            MPI_Send(              
            /* data         = */ edgeData, 
            /* count        = */ sizeof(edgeData)/sizeof(int), 
            /* datatype     = */ MPI_INT, 
            /* destination  = */ worldRank + 1, 
            /* tag          = */ worldRank + 1,             // recv side
            /* communicator = */ MPI_COMM_WORLD);
          }
          subJobs = newJobLength;

        }
      }

      //reset the jobCount, increment the commTurn
      jobCount = 0;
      commTurn ++;
    }


  }
}



void longestPalindromeSubseq(DPTable *T) {

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

  // ---------- initialize the edgeData buffer -------------------//
  // the first item in the buffer indicate how many cells are in used//
  int size = endCol - startCol + 1; // <-- this is the actual size for this rank
  // std::vector<int>* edgeData = new std::vector<int> (subCols + 2);
  // edgeData[0] = size;
  int *edgeData = (int*) malloc ((subCols + 2) * sizeof(int));
  edgeData[0] = size;


  // ----------- serial OR multi_thread OR MPI ----------------- //

  std::cout << "for process: " << worldRank << ", the start col: "<< startCol << ", the end col: " << endCol << "\n";
  // fillTableMPI(T, startCol, endCol, worldSize, worldRank);
  fillTableMPIOptimization(T, startCol, endCol, worldSize, worldRank, edgeData);



  timeTaken = serialTimer.stop();
  



  if(worldRank == worldSize - 1){
    // T->printTable();
    std::cout << "the final result is: " << T->read((T->sLength) - 1, 0) << "\n";
    std::cout << "total time using(in seconds): " 
            << timeTaken << "\n";
  }

  free(edgeData);
  MPI_Finalize();
  // return T->read((T->sLength) - 1, 0);
  return ;
}


// args: inputS
//  -- inputS: the original input string to check longest sub sequence parlindrom. default value: "CMPT431-2024FALL"


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


  longestPalindromeSubseq(DBT);
  // DBT->printTable();
  delete DBT;


  return 0;
}









