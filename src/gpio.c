/*----------------------------------------------------------------------------
    Code for Lab 4

    Configuration code for GPIO
    Functions to turn LEDs on and off
 *---------------------------------------------------------------------------*/
 
#include <MKL25Z4.h>
#include <stdbool.h>
#include "gpio.h"


/* ----------------------------------------
   Configure GPIO output for on-board LEDs 
     1. Enable clock to GPIO ports
     2. Enable GPIO ports
     3. Set GPIO direction to output
     4. Ensure LEDs are off
 * ---------------------------------------- */
void configureGPIOoutput(void) {

  // Enable clock to ports B and D
  SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK;
  
  // Make 3 pins GPIO
  PORTB->PCR[RED_LED_POS] &= ~PORT_PCR_MUX_MASK;          
  PORTB->PCR[RED_LED_POS] |= PORT_PCR_MUX(1);          
  PORTB->PCR[GREEN_LED_POS] &= ~PORT_PCR_MUX_MASK;          
  PORTB->PCR[GREEN_LED_POS] |= PORT_PCR_MUX(1);          
  PORTD->PCR[BLUE_LED_POS] &= ~PORT_PCR_MUX_MASK;          
  PORTD->PCR[BLUE_LED_POS] |= PORT_PCR_MUX(1);          
  
  // Set ports to outputs
  PTB->PDDR |= MASK(RED_LED_POS) | MASK(GREEN_LED_POS);
  PTD->PDDR |= MASK(BLUE_LED_POS);

  // Turn off LEDs
  PTB->PSOR = MASK(RED_LED_POS) | MASK(GREEN_LED_POS);
  PTD->PSOR = MASK(BLUE_LED_POS);
}

/*----------------------------------------------------------------------------
  Function that turns Red LED on or off
 *----------------------------------------------------------------------------*/
void redLEDOnOff (int onOff) {
  if (onOff == LED_ON) {
    PTB->PCOR = MASK(RED_LED_POS) ;
  } else {
    PTB->PSOR = MASK(RED_LED_POS) ;
  }
}

/*----------------------------------------------------------------------------
  Function that turns Green LED on or off
 *----------------------------------------------------------------------------*/
void greenLEDOnOff (int onOff) {
  if (onOff == LED_ON) {
    PTB->PCOR = MASK(GREEN_LED_POS) ;
  } else {
    PTB->PSOR = MASK(GREEN_LED_POS) ;
  }
}

/*----------------------------------------------------------------------------
  Function that turns Blue LED on or off
 *----------------------------------------------------------------------------*/
void blueLEDOnOff (int onOff) {  
  if (onOff == LED_ON) {
    PTD->PCOR = MASK(BLUE_LED_POS) ;
  } else {
    PTD->PSOR = MASK(BLUE_LED_POS) ;
  }
}

