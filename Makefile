
CC=clang
CLSPV=clspv
LIB=-lvulkan

vkcl: vkcl.h vkcl.c vkcl.cl
	$(CLSPV) -O=2 vkcl.cl -o vkcl.spv -descriptormap=vkcl.map
	$(CC) -o vkcl vkcl.c $(INC) $(LIB)

clean:
	rm -f vkcl
