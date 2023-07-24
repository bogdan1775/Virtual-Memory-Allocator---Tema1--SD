# Copyright Croitoru Constantin-Bogdan Grupa 314CA 2022-2023
# compiler setup
CC=gcc
CFLAGS=-Wall -Wextra -std=c99 

# define targets
TARGETS=vma

build:
	$(CC) $(CFLAGS) *.c *.h -o vma -g

pack:
	zip -FSr 314CA_CroitoruConstantin-Bogdan_Tema1.zip README Makefile *.c *.h

clean:
	rm -f $(TARGETS)

.PHONY:pack clean
