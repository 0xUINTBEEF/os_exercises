# Operating System Concepts Visualization

A comprehensive collection of operating system programming exercises focusing on process management, thread synchronization, and concurrent programming implementations.

## Project Structure

- **process/** - Process management and IPC implementations
  - Process creation and scheduling examples
  - Inter-process communication using pipes and shared memory
  - ✅ Zombie process management and prevention
  - Process isolation techniques
  - OS book programming exercises

- **synchronization/** - Thread synchronization mechanisms
  - ✅ Monitor implementation with priority inheritance
  - ✅ Dining philosophers problem solutions
  - ✅ Readers-writers problem implementation
  - ✅ Peterson's algorithm implementation
  - Deadlock detection and prevention
  - Barrier synchronization
  - Semaphore implementations

- **threads/** - Thread programming exercises
  - ✅ Thread pool implementation
  - ✅ Factorial and Fibonacci calculations using pthreads
  - ✅ Thread priority management
  - ✅ Thread scheduling visualization
    - ✅ Real-time display of thread states
    - ✅ Priority-based scheduling visualization
    - Context switching animation
  - Thread-local storage examples
  - Thread-safe data structures
  - OS book thread programming exercises

- **ui/** - User interface
  - ✅ Terminal-based menu system
  - ✅ Interactive visualization of OS concepts
  - ASCII art visualizations
  - Performance metrics display

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
  - Dining philosophers
  - Readers-writers
  - Producer-consumer

### Thread Management
- Thread creation and scheduling
- Thread pool for task management
- Priority-based thread execution
- Thread scheduling visualization
- Performance measurement tools

### Process Management
- Process creation and termination
- Process scheduling algorithms
- Zombie process prevention
- Process isolation techniques
- IPC mechanisms
  - Pipes
  - Shared memory
  - Message queues

## Planned Features

### Thread Management Enhancements
- ✅ Thread scheduling visualization
  - ✅ Real-time display of thread states
  - ✅ Priority-based scheduling visualization
  - Context switching animation
- Thread performance analysis
  - CPU utilization metrics
  - Memory usage tracking
  - I/O operation monitoring
- Advanced thread patterns
  - Thread pool with dynamic scaling
  - Work stealing scheduler
  - Thread affinity management

### Process Management Improvements
- Process scheduling algorithms
  - Round Robin implementation
  - Priority scheduling
  - Multilevel feedback queue
- Process visualization
  - Process state diagram
  - Resource allocation graph
  - Process tree visualization
- Advanced IPC mechanisms
  - Message passing interface
  - Remote procedure calls
  - Distributed shared memory

### Synchronization Extensions
- Advanced synchronization primitives
  - Read-write locks
  - Barrier implementation
  - Spin locks
- Deadlock handling
  - Prevention algorithms
  - Avoidance techniques
  - Detection and recovery
- Performance optimization
  - Lock-free algorithms
  - Wait-free synchronization
  - Memory consistency models

### Memory Management
- Memory allocation strategies
  - Buddy system
  - Slab allocator
  - Memory pools
- Page replacement algorithms
  - FIFO
  - LRU
  - Clock algorithm
- Virtual memory simulation
  - Page table visualization
  - TLB simulation
  - Page fault handling

### File System Simulation
- File system operations
  - File allocation methods
  - Directory structure
  - File access control
- Disk scheduling algorithms
  - FCFS
  - SSTF
  - SCAN/C-SCAN
- File system visualization
  - Disk layout
  - File allocation table
  - Directory tree

### Network and Distributed Systems
- ✅ Socket programming examples
  - ✅ TCP/IP implementation
  - ✅ UDP communication
  - ✅ Multicast messaging
- Distributed algorithms
  - Leader election
  - Mutual exclusion
  - Consensus protocols
- Network visualization
  - Connection graphs
  - Protocol state machines
  - Packet flow diagrams

### Security Features
- Access control mechanisms
  - Capability-based security
  - Role-based access control
  - Mandatory access control
- Security protocols
  - Authentication
  - Authorization
  - Encryption
- Security visualization
  - Access matrix
  - Security domains
  - Attack graphs

### Testing and Debugging
- Automated testing framework
  - Unit tests
  - Integration tests
  - Performance tests
- Debugging tools
  - Thread state inspection
  - Memory leak detection
  - Deadlock detection
- Performance profiling
  - CPU profiling
  - Memory profiling
  - I/O profiling

### Documentation and Education
- Interactive tutorials
  - Step-by-step guides
  - Concept explanations
  - Code walkthroughs
- Educational resources
  - Theory explanations
  - Practice exercises
  - Solution guides
- API documentation
  - Function references
  - Usage examples
  - Best practices

## Building and Testing

### Prerequisites
- GCC compiler
- POSIX threads library (pthread)
- Make build system
- ncurses library (for UI)

### Compilation
```bash
# Compile all projects
make all

# Compile specific components
cd synchronization && make
cd threads && make
cd process && make
cd ui && make
```

### Running Tests
```bash
# Test monitor implementation
./synchronization/test_monitor

# Run dining philosophers simulation
./synchronization/dining_philosophers

# Run thread pool example
./threads/thread_pool

# Run process scheduling example
./process/process_scheduling
```

### Running the UI
```bash
cd ui
./menu
```

## Performance Metrics

The test suite provides detailed performance metrics including:
- Operation throughput
- Average response time
- Success/failure rates
- Timeout statistics
- Priority inversion incidents
- Deadlock detection
- Resource utilization

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
