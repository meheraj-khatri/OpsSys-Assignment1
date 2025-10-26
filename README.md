# 🧵 Producer-Consumer Problem: Shared Memory IPC

This project implements the classic **Producer-Consumer Problem** using **Inter-Process Communication (IPC)** features available in Linux/Unix, specifically **Shared Memory** and **Named POSIX Semaphores**.

## 1. Program Description

The assignment requires two separate programs: `producer` and `consumer`. [cite_start]They communicate via a shared circular buffer (the "table") that can hold a maximum of **2 items** (`BUFFER_SIZE = 2`).

* The **Producer** generates random integers and deposits them into the shared buffer. [cite_start]It waits if the buffer is full.
* The **Consumer** retrieves and removes items from the buffer. [cite_start]It waits if the buffer is empty.
* [cite_start]Both programs run concurrently as separate processes and synchronize their access to the shared buffer using semaphores to ensure **mutual exclusion**  and proper order.
* Each program produces/consumes a total of **10 items** (`MAX_ITEMS = 10`).

## 2. Usage Instructions

### Environment

[cite_start]This project must be compiled and executed on a **Linux/Unix** environment using C/C++[cite: 7]. Ensure the necessary libraries for pthreads and real-time extensions are available.

### Compilation

[cite_start]Compile both files using the `gcc` command, linking with the **pthread** (`-pthread`) and **real-time** (`-lrt`) libraries[cite: 17, 18]:

```bash
$gcc producer.c -pthread -lrt -o producer$ gcc consumer.c -pthread -lrt -o consumer