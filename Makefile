
CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -Iinclude -O2
DEBUG_FLAGS := -Wall -Wextra -std=c11 -Iinclude -g -O0 -DDEBUG
 
SRC := src/bitstream.c src/hashchain.c src/lz77_compress.c src/lz77_decompress.c
OBJ := $(SRC:src/%.c=bin/%.o)
 
.PHONY: all clean test debug
 
all: bin/compress.exe bin/decompress.exe
 
bin/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
 
bin/compress.exe: bin/compress.o $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@
 
bin/decompress.exe: bin/decompress.o $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@
 
bin/compress.o: bin/compress.c
	$(CC) $(CFLAGS) -c $< -o $@
 
bin/decompress.o: bin/decompress.c
	$(CC) $(CFLAGS) -c $< -o $@
 
test: bin/roundtrip_test.exe
	./bin/roundtrip_test.exe
 
bin/roundtrip_test.exe: tests/roundtrip_test.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@
 
debug:
	$(MAKE) CFLAGS="$(DEBUG_FLAGS)" all
 
clean: