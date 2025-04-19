# Pipe Communication Examples

This directory contains examples of inter-process communication using pipes in Unix-like operating systems.

## Examples

### Bidirectional Communication (`bidirectional.c`)

Demonstrates bidirectional communication between parent and child processes using two pipes:
- One pipe for parent-to-child communication
- One pipe for child-to-parent communication

Features:
- Bidirectional communication
- Error handling
- Graceful shutdown
- Safe buffer management
- Signal handling
- Resource cleanup

### Named Pipe (FIFO) Communication (`named_pipe.c`)

Demonstrates communication between parent and child processes using a named pipe (FIFO):
- Named pipe creation and management
- One-way communication
- File system persistence

Features:
- Named pipe creation and cleanup
- Error handling
- Graceful shutdown
- Safe buffer management
- Signal handling
- Resource cleanup

### Unidirectional Pipe Communication (`unidirectional.c`)

Demonstrates one-way communication between parent and child processes using an unnamed pipe:
- Simple pipe creation
- One-way communication
- Process synchronization

Features:
- Unidirectional communication
- Error handling
- Graceful shutdown
- Safe buffer management
- Resource cleanup

## Building and Running

To build all examples:

```bash
make
```

To run all examples:

```bash
make test
```

To clean up:

```bash
make clean
```

## Requirements

- GCC compiler
- POSIX-compliant operating system
- Make build system

## Notes

- All examples demonstrate proper resource management and error handling
- Signal handling is implemented for graceful shutdown
- Buffer sizes are defined as constants for easy modification
- All file descriptors are properly closed
- Process status is checked and reported
- Named pipes persist in the file system until explicitly removed
