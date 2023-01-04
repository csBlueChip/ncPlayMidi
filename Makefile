DEV = /dev/null
MID = mid/TubularBells1XG.mid

EXE = midp
SRC = $(wildcard midp_*.c)
OBJ = $(SRC:.c=.o)
HDR = $(wildcard midp_*.h)
LIB = -lasound
DBG = -g

.phony: all
all : $(EXE)

$(EXE) : $(SRC) $(HDR) $(OBJ)
	rm -f midp
	gcc $(OBJ) $(LIB) $(DBG) -o $(EXE)

.phony: clean
clean :
	rm -f $(EXE) $(OBJ)

%.o : %.c
	gcc -c $(DBG) $< -o $@

.phony: play
play:
	./$(EXE) $(MID) $(DEV)

.phony: dump
dump:
	./$(EXE) $(MID)

