CC=gcc
MPICC=mpicc
#CFLAGS=-O3

default: all

point_distributing:
	$(MPICC) -o point_distributing point_distributing.c	

.PHONY: clean

all: point_distributing

clean:
	rm -f point_distributing
