OS Shell Project
Overview
The OS Shell Project is a comprehensive simulation of a Unix-like operating system shell, implemented in the C programming language. This project covers essential operating system functionalities, including process scheduling, memory management, and command execution. The project is divided into three phases, each adding more complexity and features to the shell.

Features
Phase 1: Basic Shell Functionality
Basic Shell Commands: Implemented fundamental shell commands such as help, quit, set, print, and run.
Enhanced set Command: Extended the set command to support multiple alphanumeric tokens (up to 5) for variable values.
Echo Command: Added the echo command to display strings or variable values directly from the command line.
Batch Mode Execution: Improved batch mode execution to avoid infinite loops and refined the output format for better readability.
File and Directory Operations: Implemented commands like ls, mkdir, touch, and cd to manage files and directories within the shell.
Phase 2: Multi-process Scheduling
Concurrent Process Execution: Extended the shell to support concurrent execution of multiple scripts.
Scheduling Policies: Implemented various scheduling policies, including First-Come, First-Serve (FCFS), Shortest Job First (SJF), Round Robin (RR), and Aging.
Process Control Block (PCB): Designed and implemented PCBs to manage process information and states.
Ready Queue: Created a ready queue to manage processes waiting for execution.
Phase 3: Memory Management
Demand Paging: Introduced demand paging to handle processes that exceed the shell memory limits, simulating a virtual memory system.
Least Recently Used (LRU) Policy: Implemented the LRU page replacement policy to manage memory more efficiently.
Backstore Management: Simulated a backing store to hold pages not currently in main memory.
Dynamic Memory Allocation: Enhanced the shell to dynamically allocate memory for processes and manage memory fragmentation.
