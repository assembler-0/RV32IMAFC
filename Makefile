CC = ccache clang
CFLAGS = -Wall -Wextra -g

SRC = $(wildcard src/*.c src/core/*.c)
OBJ = $(SRC:.c=.o)

TARGET = riscv
TEST_SRC = tests/test_rv32im.c
TEST_OBJ = $(TEST_SRC:.c=.o)
TEST_TARGET = test_runner

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $(TEST_TARGET) $(OBJ) $(TEST_OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)
