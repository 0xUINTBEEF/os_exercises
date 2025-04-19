CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = 

SRCS = bidirectional.c named_pipe.c unidirectional.c
OBJS = $(SRCS:.c=.o)
TARGETS = $(SRCS:.c=)

.PHONY: all clean test

all: $(TARGETS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%: %.o
	$(CC) $(LDFLAGS) $< -o $@

clean:
	rm -f $(OBJS) $(TARGETS)

test: all
	@echo "Testing bidirectional pipe..."
	./bidirectional
	@echo "\nTesting named pipe..."
	./named_pipe
	@echo "\nTesting unidirectional pipe..."
	./unidirectional 