CC = gcc
CFLAGS = -Wall -Wextra -Werror # --std=c11
PNAME = observer
SRC = $(PNAME).c
OUT = $(PNAME)


all: build_run

build:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

build_run: build
	./$(OUT)

valgrind:
	valgrind --leak-check=full -s ./$(OUT)
