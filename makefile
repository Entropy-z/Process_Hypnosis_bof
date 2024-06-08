MAKEFLAGS = -s 

CCX64 = x86_64-w64-mingw32-gcc
INC   = -I Include

hypnosis:
	$(CCX64) $(INC) -w -c Source/hypnosis.c -o dist/hypnosis.x64.o