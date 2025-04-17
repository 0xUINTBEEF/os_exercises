# Operating System Concepts Examples and Explanations

This repository is a collection of examples and explanations of OS concepts, written in C.

## Directory Structure

- `threads/`: Examples demonstrating thread concepts
  - Thread creation and management
  - Thread synchronization
  - Thread-safe operations

- `process/`: Examples demonstrating process concepts
  - Process creation and management
  - Inter-Process Communication (IPC)
  - Process scheduling

- `synchronization/`: Examples demonstrating synchronization concepts
  - Mutual exclusion
  - Semaphores
  - Deadlock prevention

## Building and Running

Each directory contains its own Makefile for building and running examples.

### Requirements

- C compiler (gcc)
- POSIX threads library
- Make
- CMake (for some examples)

### Building

```bash
# Build all examples
make

# Build specific examples
make -C threads
make -C process
make -C synchronization
```

### Running Tests

```bash
make test
```

## Resources

- [Operating System Concepts (10th Edition)](https://codex.cs.yale.edu/avi/os-book/OS10-global/)
- [POSIX Threads Programming](https://computing.llnl.gov/tutorials/pthreads/)
- [Linux System Programming](http://shop.oreilly.com/product/0636920026892.do)

## Contribution

For adding new examples or fixing existing ones, please follow the [contribution guidelines](CONTRIBUTING.md).

## License

This project is licensed under the MIT License - see the LICENSE file for details.
