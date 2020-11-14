# Given code for lab 4


The project uses a serial data link (over USB) to/from a terminal emulator running on the
connected PC / laptop, to control the speeds at which the LED light colour alternates between Green and Red.

The behaviour is: 
  * the LED colour switches between Red and Green with equal on times
  * The on times can be made faster or slower using command entered on the terminal
    emulator. The command strings are `faster` and `slower`
  * Faster command works as follows Faster:	4s	→ 3.5s	→ …	→ 0.5s	→ 4	s	→ 3.5	s
  * Slower command works as follows Slower:	0.5	s	→ 1s	→ 1.5s	→ …	→ 4s	→ 0.5	s
  * the system is initialised in the GREENON state
  * if the new on-time is yet to be completed when a command is entered the LED will immediately be given the new on-time 
  * if the new on-time has already expired when a command is entered the LED that is lit changes immediately 
 

The project uses:
 * Two threads
 * A message queue
 

