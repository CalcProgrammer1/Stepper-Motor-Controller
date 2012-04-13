/*---------------------------------------------*\
|  RGB Fan Controller - Serial Port Header File	|
| Implements a buffered serial I/O port using 	|
|  Serial on the ATMega168 and compatible chips	|
|												|
|  Adam Honse (CalcProgrammer1), Feb. 2010		|
|												|
|  This code may be freely reused for non-		|
|  commercial use.								|
\*---------------------------------------------*/
#ifndef SERIAL_H
#define SERIAL_H

#include <avr/io.h>
#include <avr/interrupt.h>

// Serial_Init
// Initializes the serial port
// Takes an AVR USART speed (see the ATMega168 datasheet)
void serial_init(unsigned int baud);

// Serial_Transmit
// Transmits a single byte of data
void serial_transmit(unsigned char data);

// Serial_Available
// Returns number of available bytes in buffer
unsigned char serial_available();

// Serial_Read
// Buffered read on Serial
// Pulls the first byte from the serial port buffer and deletes it
unsigned char serial_read();

// Serial_Flush
// Flush the Serial buffer
// Deletes any data currently in the read buffer
void serial_flush();

// Serial Print
// Prints a string
void serial_print(char * text);

#endif
