CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lrt

SRCS = shared_mem_writer.c shared_mem_reader.c
OBJS = $(SRCS:.c=.o)
TARGETS = writer reader

.PHONY: all clean test

all: $(TARGETS)

writer: shared_mem_writer.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

reader: shared_mem_reader.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGETS)

test: all
	@echo "Running shared memory tests..."
	@echo "1. Starting writer process..."
	@./writer &
	@sleep 1
	@echo "2. Starting reader process..."
	@./reader
	@echo "Tests completed." 