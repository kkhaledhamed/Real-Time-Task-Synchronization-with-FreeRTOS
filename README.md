# Real-Time-Task-Synchronization-with-FreeRTOS

## Project Overview
This project simulates an embedded system with multiple tasks, including three sender tasks and one receiver task. The tasks communicate using a queue, with semaphores and timers facilitating synchronization. The main goal of the project is to optimize the synchronization between sender and receiver tasks to minimize the number of blocked messages and improve system performance.

## System Design
### Main Structures
Task Handles: Manage sender and receiver tasks.
Semaphores: Synchronize tasks with their respective timers.
Queue: A shared queue with a size of 3 or 10, used to store messages from the sender tasks.
Timers: Periodically release semaphores to trigger sender and receiver tasks.
Functions:
Reset Function: Resets counters, prints statistics, and handles end-of-game conditions.
Random Number Generator: Generates random numbers using a uniform distribution.
Initialization Function: Initializes tasks, queue, semaphores, and timers.

### Tasks
Sender Tasks:
Three sender tasks are implemented.
Each task waits for its semaphore, checks if the queue has space, and sends a message.
If the queue is full, the task increments a blocked message counter.
The task then changes its timer period based on a random value.
Receiver Task:
The receiver task waits for its semaphore and receives messages from the queue.
Upon receiving 1000 messages, it triggers the Reset function.

### Timer Callback Functions
Sender Timer Callbacks: Release semaphores for the corresponding sender tasks.
Receiver Timer Callback: Releases the semaphore for the receiver task and triggers the Reset function when 1000 messages have been received.
## Code Overview
### Sender Task (main.c)
Function: Manages message sending for senders.
Workflow:
Waits for a semaphore.
Attempts to send a message with the current system tick to the queue.
If successful, increments the success counter; if blocked, increments the blocked counter.
Sets a new random period for its timer.
### Receiver Task (main.c)
Function: Receives messages from the queue.
Workflow:
Waits for its semaphore, released by its timer.
Receives messages from the queue.
If a message is successfully received, increments the receive counter.
If the receive counter reaches 1000, triggers the Reset function.
### Random Number Generator (main.c)
Function: Generates random numbers using a uniform distribution.
Implementation:
Generates a range of numbers based on defined upper and lower boundaries.
### Initialization Function (main.c)
Function: Initializes tasks, queue, semaphores, and timers.
Workflow:
Creates the queue.
Initializes sender and receiver tasks.
Initializes semaphores and sets up timers for each task.

## Results and Discussion
### Number of Total Sent & Blocked Messages
Issue: A gap exists between the number of sent and received messages due to differences in task frequencies.
Solution: Optimizing synchronization and balancing the rates of sender and receiver tasks is recommended.
### Effect of Increasing Queue Size
Increasing the queue size from 3 to 10 reduces the frequency of blocked messages, improves system efficiency, and enhances the flow and overall performance.
### Performance Metrics
Queue Size 3:
More blocked messages.
Lower average sending times.
Frequent waiting by senders.
Queue Size 10:
Fewer blocked messages.
Higher average sending times.
Smoother operation overall.
### Total Messages Statistics
Detailed statistics for each iteration with queue sizes 3 and 10 are provided to evaluate the performance of the system.

## With my very special thanks to my teammate: https://github.com/SarahAbelatty
