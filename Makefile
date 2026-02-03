CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
SRC = src/allocator.c src/test.c
SRC_PHASE2 = src/allocator.c src/test_phase2.c
OUT = test_allocator
OUT_PHASE2 = test_phase2

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

test_phase2:
	$(CC) $(CFLAGS) $(SRC_PHASE2) -o $(OUT_PHASE2)

clean:
	rm -f $(OUT) $(OUT_PHASE2)
