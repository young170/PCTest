CC = gcc
EXE = pctest
INCDIR = ./include
# file-specific
SRC1 = $(EXE).c
OBJ1 = $(EXE).o
# miscellaneous
MISC = solution target

# rules
$(EXE): $(OBJ1)
	$(CC) -o $@ $^

$(OBJ1): $(SRC1)
	$(CC) -o $@ -c $< -I$(INCDIR)

# Phony Targets
.PHONY: run clean

run:
	./$(EXE) -i ./testdir -t 10 solution.c target.c

clean:
	rm -rf *.o $(EXE) $(MISC)
