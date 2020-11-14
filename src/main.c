
/*----------------------------------------------------------------------------
    Given code for Embedded Systems Lab 4 
    
    This project illustrates the use of 
      - threads
      - event flags
      - a message queue

    The behaviour is: 
        - the green LED can be turned on or off using command entered on a terminal 
          emulator (see lab sheet) and send over the USB link
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

osMessageQueueId_t controlIQ;    // id for the message queue


/*--------------------------------------------------------------
 *   Thread t_redLED
 *       Flash Red LED to show an error
 *       States are: ERRORON, ERROROFF and NOERROR
 *       A signal is set to change the error status
 *          - LED off when no error
 *          - LED flashes when there is an error
 *--------------------------------------------------------------*/



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
#define REDON (1)


enum controlMsg_t {faster, slower} ;  // type for the messages
uint32_t time[] = {500, 1000, 1500, 2000, 2500, 3000, 3500, 4000};
void greenLEDThread (void *arg) {   
    int ledState = GREENON ;
    greenLEDOnOff(LED_OFF);
    uint32_t i = 3;
    osStatus_t status;
    uint32_t counter, timer;
    while (1) {
      // wait for message from queue
      status = osMessageQueueGet(controlIQ, &i, NULL, timer);       
      if (status == osOK){
        counter = osKernelGetTickCount() - counter;  
        if (counter < time[i]){       
        timer = time[i] - counter;
        }        
        else{
         timer = 1;
        }
      }
      
      else if (status == osErrorTimeout) {
       timer = time[i];
       switch (ledState) {
      
                case GREENON:
                    counter = osKernelGetTickCount(); 
                    greenLEDOnOff(LED_ON);
                    redLEDOnOff(LED_OFF);
                    ledState = REDON ;                       
                    break ;
                    
                case REDON:  
                    counter = osKernelGetTickCount(); 
                    redLEDOnOff(LED_ON);
                    greenLEDOnOff(LED_OFF);
                    ledState = GREENON ; 
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

/* const */ char prompt[] = "Command: faster / slower>" ;
/* const */ char empty[] = "" ;

void commandThread (void *arg) {
   int i = 3; 
   char response[6] ;  // buffer for response string
    enum controlMsg_t msg ;
    osMessageQueuePut(controlIQ, &i, 0, NULL);
    bool valid ;
    while (1) {
        sendMsg(empty, CRLF) ;
        sendMsg(prompt, NOLINE) ;
        readLine(response, 6) ;  // 
        valid = true ;
        if (strcmp(response, "faster") == 0) {
            msg = faster ;
        } else if (strcmp(response, "slower") == 0) {
            msg = slower ;
        } else valid = false ;
        
        if (valid) {
             if (msg == slower) {                 
                     i = i+1;                   
                  if (i > 7){
                  i = 0;
                 }                   
        } 
                    
        else if (msg == faster) {                                                      
                    i = i-1;                   
                  if(i < 0){
                  i = 7; 
                 } 
        }           
        osMessageQueuePut(controlIQ, &i, 0, NULL);  // Send Message     
        } 
        
        else {
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
    

    
    // create message queue
    controlIQ = osMessageQueueNew(2, 1, NULL);
    // initialise serial port 
    initSerialPort() ;

    // Create threads
    t_greenLED = osThreadNew(greenLEDThread, NULL, NULL);
    t_command = osThreadNew(commandThread, NULL, NULL); 
    
    osKernelStart();    // Start thread execution - DOES NOT RETURN
    for (;;) {}         // Only executed when an error occurs
}
