all: pcm4

pcm4: byteblock.c byteblock.h main.c 
	gcc byteblock.c main.c -Wall --std=c99 -D_DEFAULT_SOURCE -lpthread -lm -o pcm4
