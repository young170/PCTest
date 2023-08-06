CC = gcc
CFLAGS = -Wall
EXE = pctest
INCLS = ./include
# file-specific
SRC1 = $(EXE).c
OBJ1 = $(EXE).o
# miscellaneous
MISC = solution target

# rules
$(EXE): $(OBJ1)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ1): $(SRC1)
	$(CC) $(CFLAGS) -o $@ -c $< -I$(INCLS)

# Phony Targets
.PHONY: run clean

run:
	./$(EXE) -i ./testdir -t 10 solution.c target.c

clean:
	rm -rf *.o $(EXE) $(MISC)
