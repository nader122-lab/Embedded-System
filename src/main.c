/*
To do:
6. Hanlde UART read errors
7. Do interrupt locks

*/
/*----------------------------------------------------------------------------
    Code for Lab 4
    
    This project illustrates the use of 
      - event flags
      - a message queue

    The behaviour is: 
        - the green LED can be turned on or off using command entered on a terminal 
          emulator (see lab sheet)data and send over the USB link
        - if the command are entered in the wrong order, the system enters an
          error state: the red LED flashes
        - to exit from the error state the 'reset' command must be entered
        - the system is initialised in the error state
        
    There are three threads
       t_serial: waits for input from the terminal; sends message to t_greenLED
       t_greenLED: turns the green LED on and off on receipt of the message. Enters
         an error state when the message is not appropriate
       t_redLED: flashes the red on and off when an error state is signalled by the
         t_greenLED
    
    Message queue: 
       * Message are put in the queue t_serial
       * Messages are read from the queue by t_greenLED

    Signal using event flags
       * A flag is set by t_greenLED
       * The thread t_redLED waits on the flag

 *---------------------------------------------------------------------------*/
 
#include "cmsis_os2.h"
#include "string.h"

#include <MKL25Z4.h>
#include <stdbool.h>
#include "gpio.h"
#include "serialPort.h"

#define RESET_EVT (1)
osEventFlagsId_t errorFlags ;       // id of the event flags
osMessageQueueId_t controlMsgQ ;    // id for the message queue


/*--------------------------------------------------------------
 *   Thread t_redLED
 *       Flash Red LED to show an error
 *       States are: ERRORON, ERROROFF and NOERROR
 *       A signal is set to change the error status
 *          - LED off when no error
 *          - LED flashes when there is an error
 *--------------------------------------------------------------*/

osThreadId_t t_redLED;        /* id of thread to flash red led */

// Red LED states
#define ERRORON (0)
#define ERROROFF (1)
#define NOERROR (2)
#define ERRORFLASH (400)

void redLEDThread (void *arg) {
    int redState = ERRORON ;
    redLEDOnOff(LED_ON);
    uint32_t flags ;                // returned by osEventFlagWait
    uint32_t delay = ERRORFLASH ;   // delay used in the wait - varies

    while (1) {
        // wait for a signal or timeout
        flags = osEventFlagsWait (errorFlags, MASK(RESET_EVT), osFlagsWaitAny, delay);
        
        // interpret state machine
        switch (redState) {
            case ERRORON:   // error state - flashing, currently on
                redLEDOnOff(LED_OFF);
                if (flags == osFlagsErrorTimeout) {  // timeout continue flashing           
                    redState = ERROROFF ;
                } else {                             // signal - move to no error state
                    redState = NOERROR ;
                    delay = osWaitForever ;
                }
                break ;

            case ERROROFF:   // error state - flashing, currently off
                if (flags == osFlagsErrorTimeout) {  // timeout continue flashing
                    redLEDOnOff(LED_ON);
                    redState = ERRORON ;
                } else {                             // signal - move to no error state
                    redState = NOERROR ;
                    delay = osWaitForever ;
                }
                break ;
                
            case NOERROR:                            // no error - react on signal
                delay = ERRORFLASH ;
                redState = ERRORON ;
                redLEDOnOff(LED_ON) ;            
                break ;
        }
    }
}

/*--------------------------------------------------------------
 *   Thread t_greenLED
 *      Set the green on and off on receipt of message
 *      Messages: on, off and reset (uses an enum)
 *      Acceptable message
 *         - in on state expect off message
 *         - in off state expect on message
 *         - in error state expect reset message
 *      If 'wrong' message received 
 *          - Move to error state
 *          - Signal red LED to flash
 *          - Reset message returns LED to on state 
 *--------------------------------------------------------------*/
osThreadId_t t_greenLED;      /* id of thread to toggle green led */

// Green LED states
#define GREENON (0)
#define GREENOFF (1)
#define GERROR (2)

enum controlMsg_t {on, off, reset} ;  // type for the messages

void greenLEDThread (void *arg) {
    int ledState = GERROR ;
    greenLEDOnOff(LED_OFF);
    enum controlMsg_t msg ;
    osStatus_t status ;   // returned by message queue get
    while (1) {
        // wait for message from queue
        status = osMessageQueueGet(controlMsgQ, &msg, NULL, osWaitForever);
        if (status == osOK) {
            switch (ledState) {
                case GREENON:
                    if (msg == off) {           // expected message
                        greenLEDOnOff(LED_OFF);
                        ledState = GREENOFF ; 
                    }
                    else {                      // unexpected - go to error
                        // signal red thread                        
                        osEventFlagsSet(errorFlags, MASK(RESET_EVT)) ;
                        greenLEDOnOff(LED_OFF);
                        ledState = GERROR ;
                    }
                    break ;
                    
                case GREENOFF:      
                    if (msg == on) {            // expected message
                        greenLEDOnOff(LED_ON);
                        ledState = GREENON ; 
                    }
                    else {                      // unexpected - go to error
                        // signal red thread                        
                        osEventFlagsSet(errorFlags, MASK(RESET_EVT)) ;
                        ledState = GERROR ;
                    }
                    break ;
                    
                case GERROR: 
                    if (msg == reset) {         // expected message
                        // signal red thread to elave error state  
                        osEventFlagsSet(errorFlags, MASK(RESET_EVT)) ;
                        greenLEDOnOff(LED_ON);
                        ledState = GREENON ;
                    }
                    // ignore other messages - already in error state
                    break ;
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
osThreadId_t t_command;        /* id of thread to receive command */

/* const */ char prompt[] = "Command: on / off / reset>" ;
/* const */ char empty[] = "" ;

void commandThread (void *arg) {
    char response[6] ;  // buffer for response string
    enum controlMsg_t msg ;
    bool valid ;
    while (1) {
        //sendMsg(empty, CRLF) ;
        sendMsg(prompt, NOLINE) ;
        readLine(response, 5) ;  // 
        valid = true ;
        if (strcmp(response, "on") == 0) {
            msg = on ;
        } else if (strcmp(response, "off") == 0) {
            msg = off ;
        } else if (strcmp(response, "reset") == 0) {
            msg = reset ;
        } else valid = false ;
        
        if (valid) {
            osMessageQueuePut(controlMsgQ, &msg, 0, NULL);  // Send Message
        } else {
            sendMsg(response, NOLINE) ;
            sendMsg(" not recognised", CRLF) ;
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

int main (void) { 
    
    // System Initialization
    SystemCoreClockUpdate();

    // Initialise peripherals
    configureGPIOoutput();
    //configureGPIOinput();
    init_UART0(115200) ;

    // Initialize CMSIS-RTOS
    osKernelInitialize();
    
    // Create event flags
    errorFlags = osEventFlagsNew(NULL);
    
    // create message queue
    controlMsgQ = osMessageQueueNew(2, sizeof(enum controlMsg_t), NULL) ;

    // initialise serial port 
    initSerialPort() ;

    // Create threads
    t_redLED = osThreadNew(redLEDThread, NULL, NULL); 
    t_greenLED = osThreadNew(greenLEDThread, NULL, NULL);
    t_command = osThreadNew(commandThread, NULL, NULL); 
    
    osKernelStart();    // Start thread execution - DOES NOT RETURN
    for (;;) {}         // Only executed when an error occurs
}
