# Operating System Exercises

A comprehensive collection of operating system programming exercises focusing on process management, thread synchronization, and concurrent programming implementations.

## Project Structure

- **process/** - Process management and IPC implementations
  - Process creation and scheduling examples
  - Inter-process communication using pipes and shared memory
  - OS book programming exercises

- **synchronization/** - Thread synchronization mechanisms
  - Monitor implementation with priority inheritance
  - Dining philosophers problem solutions
  - Readers-writers problem implementation
  - Peterson's algorithm implementation
  - Semaphore implementations

- **threads/** - Thread programming exercises
  - Thread pool implementation
  - Factorial and Fibonacci calculations using pthreads
  - OS book thread programming exercises

## Key Features

### Monitor Implementation
- Thread-safe buffer operations with timeout support
- Priority inheritance to prevent priority inversion
- Deadlock detection capabilities
- Comprehensive performance metrics
  - Response time tracking
  - Operation success/failure monitoring
  - Timeout statistics

### Synchronization Primitives
- Mutex and condition variables
- Semaphore implementation
- Peterson's algorithm for mutual exclusion
- Solutions to classic synchronization problems

### Thread Management
- Thread creation and scheduling
- Thread pool for task management
- Priority-based thread execution
- Performance measurement tools

## Building and Testing

### Prerequisites
- GCC compiler
- POSIX threads library (pthread)
- Make build system

### Compilation
```bash
# Compile all projects
make all

# Compile specific components
cd synchronization && make
cd threads && make
cd process && make
```

### Running Tests
```bash
# Test monitor implementation
./synchronization/test_monitor

# Run dining philosophers simulation
./synchronization/dining_philosophers
```

## Performance Metrics

The test suite provides detailed performance metrics including:
- Operation throughput
- Average response time
- Success/failure rates
- Timeout statistics
- Priority inversion incidents

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
