HOSTCXX = g++
TARGETCXX = aarch64-linux-gnu-g++
CXXFLAGS = -Wall -Werror -Wpedantic -std=c++17 -O0
HOSTBINARY = TempHumidLogger-x86
TARGETBINARY = TempHumidLogger-aarch64
SOURCE = main.cpp DataLogger.h HumiditySensor.h SharedResources.h TemperatureSensor.h

all: $(HOSTBINARY) $(TARGETBINARY)

$(HOSTBINARY): $(SOURCE)
	mkdir -p ./build
	$(HOSTCXX) $(CXXFLAGS) $? -o ./build/$@

$(TARGETBINARY): $(SOURCE)
	mkdir -p ./build
	$(TARGETCXX) $(CXXFLAGS) $? -o ./build/$@

clean:
	rm -rf build
