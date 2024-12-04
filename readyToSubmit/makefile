# Define compilers
MPICXX = mpicxx
CXX = g++

# Define compilation flags
CXXFLAGS = -O2
LDFLAGS_PARALLEL = -pthread

# Define target executable names
TARGET_mpi = LSSP_mpi
TARGET_serial = LSSP_serial
TARGET_parallel = LSSP_parallel

# Define source files for different targets
SRC_mpi = LSSP_MPI_modular.cpp
SRC_serial = LSSP_serial.cpp
SRC_parallel = LSSP_parallel_modular.cpp

# Define object files for each target
OBJ_mpi = $(SRC_mpi:.cpp=.o)
OBJ_serial = $(SRC_serial:.cpp=.o)
OBJ_parallel = $(SRC_parallel:.cpp=.o)

# Default target to build all executables
all: $(TARGET_mpi) $(TARGET_serial) $(TARGET_parallel)

# Rule to build the MPI executable
$(TARGET_mpi): $(OBJ_mpi)
	$(MPICXX) $(CXXFLAGS) -o $@ $^

# Rule to build the serial executable
$(TARGET_serial): $(OBJ_serial)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to build the parallel executable with pthreads
$(TARGET_parallel): $(OBJ_parallel)
	$(CXX) $(CXXFLAGS) $(LDFLAGS_PARALLEL) -o $@ $^

# Rule to compile .cpp files to .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJ_mpi) $(TARGET_mpi) $(OBJ_serial) $(TARGET_serial) $(OBJ_parallel) $(TARGET_parallel)
