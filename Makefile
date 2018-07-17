
CC=clang
INC=-I/usr/include/vulkan/
LIB=-lvulkan

vkcl: vkcl.h vkcl.c
	$(CC) -o vkcl vkcl.c $(INC) $(LIB)

clean:
	rm -f vkcl
