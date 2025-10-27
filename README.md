#### Programming Assignment #1: Producer-Consumer Problem
This repository contains the solution for the Producer-Consumer Problem 1implemented using two separate, synchronized C programs in a Linux/Unix environment


### Program Description
This assignment implements the classic Producer-Consumer Problem where two independent processes, a Producer and a Consumer, share a bounded buffer (the "table")
# Producer: Generates items and puts them onto the table
# Consumer: Picks up items from the table
# Constraint: The table can only hold two items at the same time

The system uses concurrency tools to ensure correct synchronization:
The Producer waits when the table is full (completed)
The Consumer waits when the table has no items

## Files Included

a. producer.c
The main program that creates the shared memory and semaphores, and runs the producing thread.

b. consumer.c
The main program that links to the shared resources and runs the consuming thread.

c. global.h
Header file defining shared constants (BUFFER_SIZE, resource names) and the SharedBuffer structure.

d. README.md
This file, providing program documentation, usage instructions, and example results.

## Usage Instructions
The programs are designed to be compiled and run in a Linux/Unix environment.

1. Environment Setup

Operating System: Linux/Unix is required. If you are not using Linux, you should use VirtualBox or Docker.

Compiler: Ensure gcc is installed.

Libraries: The code requires the pthreads library and the real-time library for POSIX semaphores and shared memory. These are linked using the flags -pthread and -lrt.

2. Compilation

Compile both source files using the specified flags:

# Compile producer.c 
$ gcc producer.c -pthread -lrt -o producer

# Compile consumer.c
$ gcc consumer.c -pthread -lrt -o consumer


3. Execution

Execute both programs concurrently. The Producer must start first to initialize the shared resources (memory and semaphores).Bash# Execute both programs concurrently.

$ ./producer & ./consumer &

Manual termination via Ctrl+C or killall.