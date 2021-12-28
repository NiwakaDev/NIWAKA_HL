.PHONY:hl

CC = clang

hl : 
	$(CC) -g -O0 -o hl hl.c