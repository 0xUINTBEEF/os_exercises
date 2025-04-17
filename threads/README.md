# Thread Examples

This directory contains examples demonstrating various thread concepts in C.

## Examples

1. **factorial_pthread.c**
   - Demonstrates parallel computation of factorial using multiple threads
   - Shows basic thread creation and joining
   - Implements thread-safe operations

2. **fibonacci_pthread.c**
   - Implements parallel Fibonacci number calculation
   - Shows thread synchronization
   - Demonstrates shared memory access

## Building

To build all examples:
```bash
make
```

To build a specific example:
```bash
make factorial_pthread
make fibonacci_pthread
```

## Running

To run an example:
```bash
./factorial_pthread
./fibonacci_pthread
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
