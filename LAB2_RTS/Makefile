CXX=g++
CXXFLAGS=-ggdb3 -O0
SOURCE=sensor_data_processing.cpp
TARGET=sensor_data_proccesing

all:
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(TARGET)
 
clean:
	@rm -f $(TARGET) *.out.* vgcore.*

callgrind:
	@valgrind --tool=callgrind --dump-instr=yes --cache-sim=yes --instr-atstart=no ./$(TARGET)
