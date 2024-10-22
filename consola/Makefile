SRC_DIR = ./src
_SRC = $(shell find ./src -type f -iname "*.c" -exec basename {} \;)
SRC = $(patsubst %,$(SRC_DIR)/%,$(SRC))

ODIR = ./obj
_OBJ = $(_SRC:.c=.o)
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

HEADERS = $(shell find ./include/*.h)

O_DIR = ./obj
CC = gcc

CFLAGS = -c

LINK = -lcommons -lreadline -lpthread -lncurses -lm

OUT_NAME = mainConsola.out

.PHONY: makeFolder

SHARED= $(shell find ../shared/obj/*.o)

build: makeFolder makeshared $(OBJ)
	$(CC) main.c -o $(OUT_NAME) $(OBJ) $(LINK) $(SHARED)

./obj/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $< $(LINK)

makeshared: 
	make -C ../shared

makeFolder:
	@if [ ! -d ./obj ]; then \
		mkdir obj; \
	fi

debug: CFLAGS := $(CFLAGS) -g
debug: build

valgrind: debug build
	valgrind --leak-check=full --track-origins=yes --log-file=valgrind.log ./$(OUT_NAME)

clean:
	rm -rf obj
	rm -rf *.out
	rm -rf *.log
