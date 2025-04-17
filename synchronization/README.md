# Synchronization Examples

This directory contains examples demonstrating various synchronization concepts in C.

## Examples

1. **peterson.c**
   - Implements Peterson's algorithm for mutual exclusion
   - Demonstrates software-based synchronization
   - Shows how to prevent race conditions

2. **Semaphore Examples**
   - Contains examples of semaphore usage
   - Demonstrates binary and counting semaphores
   - Shows producer-consumer problem solution

## Building

To build all examples:
```bash
make
```

To build a specific example:
```bash
make peterson
```

## Running

To run an example:
```bash
./peterson
```

## Testing

To run tests:
```bash
make test
```

## Requirements

- C compiler (gcc)
- POSIX threads library
- Make 