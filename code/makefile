SRC = $(wildcard src/*.cpp)
OUT = bin/nserial
INCLUDES = -I./include/
CCFLAGS = -Wall
CCC = g++

all: $(SRC)
	mkdir -p bin
	$(CCC) $(INCLUDES) $(CCFLAGS) $(SRC) -o $(OUT)

