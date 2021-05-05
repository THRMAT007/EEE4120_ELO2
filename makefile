# This Makefile requires GNU make, which is called gmake on Solaris systems
#
# 'make'        : builds and runs the project
# 'make clean'  : remove build products

# where the executable program binary is placed:
PROG = bin/*
# Where the object files are placed:
OBJS = obj/*

# Which compiler to use, note for MPI a special purpose compiler is used
CC = mpic++

# Precompiled libraries to link in:
LDLIBS  = -L/usr/lib/openmpi/lib -L/usr/lib -lm -ljpeg -lmpi
# Included H files needed during compiling:
INCLUDE = -ITools -I/usr/lib/openmpi/include

.PHONY: clean ELO2 run

all: clean ELO2 run

clean:
	rm -f -r $(PROG) $(OBJS)

ELO2:
	mkdir -p bin/ obj/
	$(CC) $(INCLUDE) -c ELO2.cpp -o obj/ELO2.o
	$(CC) $(INCLUDE) -c Tools/JPEG.cpp -o obj/JPEG.o
	$(CC) $(INCLUDE) -c Tools/Timer.cpp -o obj/Timer.o
	$(CC) -o bin/ELO2 obj/ELO2.o obj/JPEG.o obj/Timer.o $(LDLIBS)

# you can type "make run" to execute the program in this case with a default of 4 nodes
run:
	mpirun -np 4 bin/ELO2 


