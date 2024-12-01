MPICXX = mpicxx

TARGET_mpi = LSSP_mpi
TARGET_serial = LSSP_serial

SRC_mpi = LSSP_MPI_modular.cpp
SRC_serial = LSSP_serial.cpp

OBJ_mpi = $(SRC_mpi:.cpp=.o)
OBJ_serial = $(SRC_serial:.cpp=.o)

CXXFLAGS = -O2

all: LSSP_mpi LSSP_serial $(TARGET)

LLSP_mpi : $(TARGET_mpi)

LLSP_serial : $(TARGET_serial)


$(TARGET_mpi): $(OBJ_mpi)
	$(MPICXX) $(CXXFLAGS) -o $@ $^

$(TARGET_serial): $(OBJ_serial)
	$(MPICXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(MPICXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_mpi) $(TARGET_mpi) $(OBJ_serial) $(TARGET_serial)
