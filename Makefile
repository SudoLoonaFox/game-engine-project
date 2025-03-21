CC=gcc
CFLAGS=-g -Wall -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lGLEW -lm -fsanitize=address
SRC=src
SRCS=$(wildcard $(SRC)/*.c)
OBJ=obj
OBJS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
BIN=game

TEST=tests
TESTS=$(wildcard $(TEST)/*.c)
TESTBINS=$(patsubst $(TEST)/%.c, $(TEST)/bin/%, $(TESTS))

all:$(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ):
	mkdir $@

test: $(TEST)/bin $(TESTBINS) $(OBJS)
	for test in $(TESTBINS) ; do ./$$test ; done


$(TEST)/bin/%: $(TEST)/%.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ -lcriterion

$(TEST)/bin:
	mkdir $@

clean:
	trash-put obj/* $(TEST)/bin/* $(BIN)
