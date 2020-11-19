
/*----------------------------------------------------------------------------
    Given code for Embedded Systems Lab 4 
    
    This project illustrates the use of 
      - threads
      - a message queue

    The behaviour is: 
        -the LED colour switches between Red and Green with equal on times
        -The on times can be made faster or slower using command entered on the terminal emulator. 
         The command strings are faster and slower
        -Faster command works as follows Faster: 4s ? 3.5s ? … ? 0.5s ? 4 s ? 3.5 s
        -Slower command works as follows Slower: 0.5 s ? 1s ? 1.5s ? … ? 4s ? 0.5 s
        -the system is initialised in the GREENON state
        -if the new on-time is yet to be completed when a command is entered the LED will immediately be given the new on-time
        -if the new on-time has already expired when a command is entered the LED that is lit changes immediately
        
    There are three threads
       t_command: waits for input from the terminal; sends message to t_greenLED
       t_greenLED: turns the green LED on and off on receipt of the message. Enters
         an error state when the message is not appropriate
       
    
    Message queue: 
       * Message are put in the queue t_command
       * Messages are read from the queue by t_greenLED


 *---------------------------------------------------------------------------*/

#include "cmsis_os2.h"

#include "string.h"

#include <MKL25Z4.h>

#include <stdbool.h>

#include "gpio.h"

#include "serialPort.h"

#define RESET_EVT (1)

osMessageQueueId_t controlIQ; // id for the message queue
osThreadId_t t_greenRedLED; /* id of thread to toggle green led */

// LED states
#define GREENON (0)
#define REDON (1)

enum controlMsg_t {
  faster,
  slower
}; // type for the messages
uint32_t time[] = {
  500,
  1000,
  1500,
  2000,
  2500,
  3000,
  3500,
  4000
}; // array of faster slower times
void greenRedLEDThread(void * arg) {
  int ledState = GREENON; //initial Led colour
  greenLEDOnOff(LED_OFF); // initialise Led to Off state
  redLEDOnOff(LED_OFF);
  uint32_t i = 3; // index of initial switching speed 
  osStatus_t status; // returned by message queue get
  uint32_t counter, timer;
  while (1) {
    // wait for message from queue
    status = osMessageQueueGet(controlIQ, & i, NULL, timer);
    if (status == osOK) { // message received 
      counter = osKernelGetTickCount() - counter; // stores time passed in current LED state 
      if (counter < time[i]) { // determines additional time required
        timer = time[i] - counter;
      } else { // immediate LED switch
        timer = 1;
      }
    } else if (status == osErrorTimeout) { // timer finished 
      timer = time[i]; // new switching speed assigned
      switch (ledState) {

      case GREENON:
        counter = osKernelGetTickCount(); // current value of Kernal count 
        greenLEDOnOff(LED_ON); // set LED colour for current state
        redLEDOnOff(LED_OFF);
        ledState = REDON; // next state            
        break;

      case REDON:
        counter = osKernelGetTickCount(); // current value of Kernal count 
        redLEDOnOff(LED_ON); // set LED colour for current state
        greenLEDOnOff(LED_OFF);
        ledState = GREENON; // next state
        break;

      }
    }
  }
}

/*------------------------------------------------------------
 *  Thread t_command
 *      Request user command
 *      
 *
 *------------------------------------------------------------*/
osThreadId_t t_command; /* id of thread to receive command */

/* const */
char prompt[] = "Command: faster / slower>"; // emulator promtpt message
/* const */
char empty[] = "";

void commandThread(void * arg) {
  int i = 3; // index of initial switching speed
  char response[6]; // buffer for response string
  enum controlMsg_t msg;
  osMessageQueuePut(controlIQ, & i, 0, NULL);
  bool valid;
  while (1) {
    sendMsg(empty, CRLF);
    sendMsg(prompt, NOLINE);
    readLine(response, 6); // 
    valid = true;
    if (strcmp(response, "faster") == 0) {
      msg = faster;
    } else if (strcmp(response, "slower") == 0) {
      msg = slower;
    } else valid = false;

    if (valid) {
      if (msg == slower) {
        i = i + 1;
        if (i > 7) {
          i = 0;
        }
      } else if (msg == faster) {
        i = i - 1;
        if (i < 0) {
          i = 7;
        }
      }
      osMessageQueuePut(controlIQ, & i, 0, NULL); // Send Message     
    } else {
      sendMsg(response, NOLINE);
      sendMsg(" not recognised", CRLF);
    }
  }
}

/*----------------------------------------------------------------------------
 * Application main
 *   Initialise I/O
 *   Initialise kernel
 *   Create threads
 *   Start kernel
 *---------------------------------------------------------------------------*/

int main(void) {

  // System Initialization
  SystemCoreClockUpdate();

  // Initialise peripherals
  configureGPIOoutput();
  //configureGPIOinput();
  init_UART0(115200);

  // Initialize CMSIS-RTOS
  osKernelInitialize();

  // create message queue
  controlIQ = osMessageQueueNew(2, 1, NULL);
  // initialise serial port 
  initSerialPort();

  // Create threads
  t_greenRedLED = osThreadNew(greenRedLEDThread, NULL, NULL);
  t_command = osThreadNew(commandThread, NULL, NULL);

  osKernelStart(); // Start thread execution - DOES NOT RETURN
  for (;;) {} // Only executed when an error occurs
}
