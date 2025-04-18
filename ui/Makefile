CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lncurses

SRCS = menu.c
OBJS = $(SRCS:.c=.o)
EXES = $(SRCS:.c=)

.PHONY: all clean

all: $(EXES)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%: %.o
	$(CC) $(LDFLAGS) $< -o $@

clean:
	rm -f $(OBJS) $(EXES) 