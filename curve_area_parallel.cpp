#include "core/utils.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>


#define sqr(x) ((x) * (x))
#define DEFAULT_NUMBER_OF_POINTS "1000000000"
#define DEFAULT_A "2" // 2
#define DEFAULT_B "1" // 1
#define DEFAULT_RANDOM_SEED "1"


typedef struct MPImsg
{
  unsigned long subGP;
  unsigned long subCP;
  double subT;
}MpiMsg;

uint c_const = (uint)RAND_MAX + (uint)1;
inline double get_random_coordinate(uint *random_seed) {
  return ((double)rand_r(random_seed)) / c_const;  // thread-safe random number generator
}

unsigned long get_points_in_curve(unsigned long n, uint random_seed, float a, float b) {
  unsigned long curve_count = 0;
  double x_coord, y_coord;
  for (unsigned long i = 0; i < n; i++) {
    x_coord = ((2.0 * get_random_coordinate(&random_seed)) - 1.0);
    y_coord = ((2.0 * get_random_coordinate(&random_seed)) - 1.0);
    if ((a*sqr(x_coord) + b*sqr(sqr(y_coord))) <= 1.0)
      curve_count++;
  }
  return curve_count;
}

void curve_area_calculation_parallel(unsigned long n, float a, float b, uint r_seed, int worldSize, int worldRank) {





  timer serial_timer;
  timer subT;
  serial_timer.start();
  subT.start();

  double time_taken = 0.0;
  uint random_seed = r_seed;



  unsigned long subJobs = n / worldSize;
  unsigned long extraJobs = n % worldSize;


  unsigned long curve_points;
  MPImsg* subMsg = new struct MPImsg;



  if(worldRank < extraJobs){
    curve_points = get_points_in_curve(subJobs + 1, r_seed + worldRank, a, b);
    subMsg->subGP = subJobs + 1;
    
  }
  else{
    curve_points = get_points_in_curve(subJobs, r_seed + worldRank, a, b);
    subMsg->subGP = subJobs;
  }

  subMsg->subCP = curve_points;
  subMsg->subT = subT.stop();

  if(worldRank == 0){
    std::cout << "Number of points : " << n << "\n";;
    std::cout << "A : " << a << "\n" << "B : " << b << "\n";
    std::cout << "Random Seed : " << r_seed << "\n";
    std::cout << "rank, points_generated, curve_points, time_taken\n";

      
    std::cout   << "0, " << subMsg->subGP << ", "
                << subMsg->subCP << ", " << std::setprecision(TIME_PRECISION)
                << subMsg->subT << "\n";
  }



  // create a customized mpi data type

  if(worldSize > 1){
    MPI_Datatype types[3] = {MPI_UNSIGNED_LONG, MPI_UNSIGNED_LONG, MPI_DOUBLE};
    MPI_Datatype mpi_msgType;
    int blockLength[3] = {1, 1, 1};
    MPI_Aint offsets[3];

    offsets[0] = offsetof(MpiMsg, subGP);
    offsets[1] = offsetof(MpiMsg, subCP);
    offsets[2] = offsetof(MpiMsg, subT);
    
    MPI_Type_create_struct(3, blockLength, offsets, types, &mpi_msgType);
    MPI_Type_commit(&mpi_msgType);

    unsigned long received;

    if(worldRank > 0){
      MPI_Send(
        /* data         = */ subMsg, 
        /* count        = */ 1, 
        /* datatype     = */ mpi_msgType, 
        /* destination  = */ 0, 
        /* tag          = */ 0, 
        /* communicator = */ MPI_COMM_WORLD);
    }
    else{
     


      for(int i = 1; i < worldSize; i++){
        MPI_Recv(
          /* data         = */ subMsg, 
          /* count        = */ 1, 
          /* datatype     = */ mpi_msgType, 
          /* source       = */ i, 
          /* tag          = */ 0, 
          /* communicator = */ MPI_COMM_WORLD, 
          /* status       = */ MPI_STATUS_IGNORE);

        std::cout <<i  << ", " << subMsg->subGP << ", "
                  << subMsg->subCP << ", " << std::setprecision(TIME_PRECISION)
                  << subMsg->subT << "\n";

        curve_points+= subMsg->subCP;
      }


      
    }
  }
  if (worldRank == 0){
    double area_value;
    area_value =
        4.0 * (double)curve_points / (double)n;

    time_taken = serial_timer.stop();
    std::cout << "Total points generated : " << n << "\n";
    std::cout << "Total points in curve : " << curve_points << "\n";
    std::cout << "Area : " << std::setprecision(VAL_PRECISION) << area_value
              << "\n";
    std::cout << "Time taken (in seconds) : " << std::setprecision(TIME_PRECISION)
              << time_taken << "\n";
  }

  




  
  //*------------------------------------------------------------------------
  delete subMsg;
}

int main(int argc, char *argv[]) {
  MPI_Init(NULL, NULL);
  int worldSize;
  MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

  int worldRank;
  MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
  
  // Initialize command line arguments
  cxxopts::Options options("Curve_area_calculation",
                           "Calculate area inside curve a x^2 + b y ^4 = 1 using serial and parallel execution");
  options.add_options(
      "custom",
      {
          {"nPoints", "Number of points",         
           cxxopts::value<unsigned long>()->default_value(DEFAULT_NUMBER_OF_POINTS)},
	        {"coeffA", "Coefficient a",
	         cxxopts::value<float>()->default_value(DEFAULT_A)},
          {"coeffB", "Coefficient b",
           cxxopts::value<float>()->default_value(DEFAULT_B)},
          {"rSeed", "Random Seed",
           cxxopts::value<uint>()->default_value(DEFAULT_RANDOM_SEED)}
      });
  auto cl_options = options.parse(argc, argv);
  unsigned long n_points = cl_options["nPoints"].as<unsigned long>();
  float a = cl_options["coeffA"].as<float>();
  float b = cl_options["coeffB"].as<float>();
  uint r_seed = cl_options["rSeed"].as<uint>();

  // std::cout << "Number of points : " << n_points << "\n";;
  // std::cout << "A : " << a << "\n" << "B : " << b << "\n";
  // std::cout << "Random Seed : " << r_seed << "\n";

  curve_area_calculation_parallel(n_points, a, b, r_seed, worldSize, worldRank);
  MPI_Finalize();

  return 0;
}
