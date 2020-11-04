# Lab 4
Given code for lab 4

The purpose of this project is to illustrate the use of 
 1. Event flags, which are used to signal between threads
 2. A message queue, which is used to pass messages between threads 

The project uses a serial data link (over USB) to/from a terminal emulator running on the
connected PC / laptop. Coolterm is recommended and a configiration file included in the project: 
see the lab sheet for more details.

The behaviour is: 
  * The green LED can be turned on or off using command entered on the terminal
    emulator. The command strings are `on`, `off` and `reset`
  * When the LED is off, the only acceptable command is `on`; similarly when it is on, `off` is expected
  * If the command are entered in the wrong order, the system enters anerror state: the red LED flashes
  * To exit from the error state the `reset` command must be entered
  * The system is initialised in the error state

The project uses:
 * Three threads
 * Event flags
 * A message queue
 
In addition, the project includes the code for the serial interface. **You are not expected to be able to answer 
questions about this ocode.** Instead, you should understand the API, which has two functions:
   1. `sendMsg` which write a message to the terminal
   2. `readLine` which reads a line from the terminal
 
There are further details in the labsheet and examples in the project code. 