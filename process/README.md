# Process Examples

This directory contains examples demonstrating various process concepts in C.

## Examples

1. **process_creation.c**
   - Demonstrates basic process creation using fork()
   - Shows parent-child process relationship
   - Implements process communication

2. **IPC (Inter-Process Communication)**
   - Contains examples of different IPC mechanisms
   - Demonstrates shared memory, pipes, and message queues

## Building

To build all examples:
```bash
make
```

To build a specific example:
```bash
make process_creation
```

## Running

To run an example:
```bash
./process_creation
```

## Testing

To run tests:
```bash
make test
```

## Requirements

- C compiler (gcc)
- POSIX libraries
- Make 