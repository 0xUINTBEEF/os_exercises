CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = 

SRCS = process_creation.c process_scheduling.c zombie_process.c
OBJS = $(SRCS:.c=.o)
EXES = $(SRCS:.c=)

.PHONY: all clean test

all: $(EXES)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%: %.o
	$(CC) $(LDFLAGS) $< -o $@

clean:
	rm -f $(OBJS) $(EXES)

test: all
	@echo "Running tests..."
	@for exec in $(EXES); do \
		echo "Testing $$exec..."; \
		./$$exec; \
	done 