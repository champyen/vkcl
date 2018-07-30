
CC=clang
CLSPV=clspv
INC=-I. -I./stb
LIB=-lvulkan -lm

vkcl: vkcl.h vkcl.c vkcl.cl
	$(CLSPV) -O=2 vkcl.cl -o vkcl.spv -descriptormap=vkcl.map
	$(CC) -o vkcl vkcl.c $(INC) $(LIB)

clean:
	rm -f vkcl
