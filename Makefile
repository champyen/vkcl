
CC=clang
CLSPV=clspv
INC=-I/usr/include/vulkan/
LIB=-lvulkan

vkcl: vkcl.h vkcl.c vkcl.cl
	$(CLSPV) -O=2 vkcl.cl -o vkcl.spv
	$(CC) -o vkcl vkcl.c $(INC) $(LIB)

clean:
	rm -f vkcl
