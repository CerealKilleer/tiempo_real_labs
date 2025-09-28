HOSTGCC = gcc
TARGETGCC = aarch64-linux-gnu-gcc
SOURCE = test.c
BINARYX86 = ./build/exec-x86_64-linux
BINARYAARCH64 = ./build/exec-arch64_raspbpi

all:
	mkdir -p ./build
	$(HOSTGCC) $(SOURCE) -o $(BINARYX86)
	$(TARGETGCC) $(SOURCE) -o $(BINARYAARCH64)

file:
	file $(BINARYX86)
	file $(BINARYAARCH64)

exec:	
	$(BINARYX86)
	$(BINARYAARCH64)

clean:
	rm -rf ./build
