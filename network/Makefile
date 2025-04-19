CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -pthread

# Source files
SRCS = tcp_server.c tcp_client.c \
       udp_server.c udp_client.c \
       multicast_sender.c multicast_receiver.c

# Executable names
EXECS = tcp_server tcp_client \
        udp_server udp_client \
        multicast_sender multicast_receiver

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(EXECS)

# Compile rules
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link rules
tcp_server: tcp_server.o
	$(CC) $(LDFLAGS) $< -o $@

tcp_client: tcp_client.o
	$(CC) $(LDFLAGS) $< -o $@

udp_server: udp_server.o
	$(CC) $(LDFLAGS) $< -o $@

udp_client: udp_client.o
	$(CC) $(LDFLAGS) $< -o $@

multicast_sender: multicast_sender.o
	$(CC) $(LDFLAGS) $< -o $@

multicast_receiver: multicast_receiver.o
	$(CC) $(LDFLAGS) $< -o $@

# Clean up
clean:
	rm -f $(OBJS) $(EXECS)

# Test targets
test_tcp: tcp_server tcp_client
	@echo "Testing TCP communication..."
	@./tcp_server &
	@sleep 1
	@./tcp_client
	@pkill tcp_server

test_udp: udp_server udp_client
	@echo "Testing UDP communication..."
	@./udp_server &
	@sleep 1
	@./udp_client
	@pkill udp_server

test_multicast: multicast_sender multicast_receiver
	@echo "Testing Multicast communication..."
	@./multicast_receiver &
	@sleep 1
	@./multicast_sender
	@pkill multicast_receiver

.PHONY: all clean test_tcp test_udp test_multicast 