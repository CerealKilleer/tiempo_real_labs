HOSTCXX = g++
TARGETCXX = aarch64-linux-gnu-g++
CXXFLAGS = -Wall -Werror -Wpedantic -std=c++17 -O0
HOSTBINARY = prod_cons_lambda_solv_CPP17_v2_x86
TARGETBINARY = prod_cons_lambda_solv_CPP17_v2_aarch64
SOURCE = prod_consum_lambda_solved_cplus17_v2.cpp

all: $(HOSTBINARY) $(TARGETBINARY)

$(HOSTBINARY): $(SOURCE)
	mkdir -p ./build
	$(HOSTCXX) $(CXXFLAGS) $< -o ./build/$@  

$(TARGETBINARY): $(SOURCE)
	mkdir -p ./build
	$(TARGETCXX) $(CXXFLAGS) $< -o ./build/$@

clean:
	rm -rf build
