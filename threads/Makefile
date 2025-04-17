CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -pthread

SRCS = factorial_pthread.c fibonacci_pthread.c thread_pool.c
OBJS = $(SRCS:.c=.o)
EXECS = $(SRCS:.c=)

.PHONY: all clean test

all: $(EXECS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%: %.o
	$(CC) $(LDFLAGS) $< -o $@

clean:
	rm -f $(OBJS) $(EXECS)

test: all
	@echo "Running tests..."
	@for exec in $(EXECS); do \
		echo "Testing $$exec..."; \
		./$$exec; \
	done 