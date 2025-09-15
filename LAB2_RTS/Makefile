CXX=g++
CXXFLAGS=-ggdb3 -O0
SOURCE=sensor_data_processing.cpp
SOURCE_FIX=sensor_data_processing_fix.cpp
TARGET=sensor_data_processing
TARGET_FIX=sensor_data_processing_fix

all:
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(TARGET)
	$(CXX) $(CXXFLAGS) $(SOURCE_FIX) -o $(TARGET_FIX)
 
clean:
	@rm -f $(TARGET) $(TARGET_FIX) *.out.* vgcore.*

callgrind:
	@valgrind --tool=callgrind --dump-instr=yes --cache-sim=yes --instr-atstart=no ./$(TARGET)

callgrind-fix:
	@valgrind --tool=callgrind --dump-instr=yes --cache-sim=yes --instr-atstart=no ./$(TARGET_FIX)

leak-check:
	@valgrind --leak-check=full ./$(TARGET)

leak-check-fix:
	@valgrind --leak-check=full ./$(TARGET_FIX)

massif:
	@valgrind --tool=massif --peak-inaccuracy=0.0 --stacks=yes --heap=yes --time-unit=ms --detailed-freq=1 --threshold=0.0 ./${TARGET}

massif-fix:
	@valgrind --tool=massif --peak-inaccuracy=0.0 --stacks=yes --heap=yes --time-unit=ms --detailed-freq=1 --threshold=0.0 ./${TARGET_FIX}
