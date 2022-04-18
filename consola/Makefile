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

OUT_NAME = main.out

.PHONY: makeFolder

build: makeFolder $(OBJ)
	$(CC) main.c -o $(OUT_NAME) $(OBJ) $(LINK)

./obj/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $< $(LINK)


makeFolder:
	@if [ ! -d ./obj ]; then \
		mkdir obj; \
	fi

debug: CFLAGS := $(CFLAGS) -g
debug: build

clean:
	rm -rf obj
	rm -rf *.out
