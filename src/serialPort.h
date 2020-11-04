// Header file for UART
//   Function prototypes

#ifndef UART_DEFS_H
#define UART_DEFS_H

#include <MKL25Z4.h>
#include <stdbool.h>

// values used for eol parameter
#define NOLINE (0)
#define LFONLY (1)
#define CRLF (2)

void init_UART0(uint32_t baud_rate) ;
void initSerialPort(void) ;
bool sendMsg(char *msg, int eol) ;
bool readLine (char *msg, int maxChars) ; 

#endif
