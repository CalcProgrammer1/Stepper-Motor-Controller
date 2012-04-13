/*---------------------------------------------*\
|  RGB Fan Controller - Serial Port Code File	|
|  This file contains the functionality of the	|
|  buffered serial port.  It uses a buffer array|
|  and a position value.  The default buffer	|
|  size is 64 bytes, though you can increase 	|
|  it if you need a larger buffer.				|
|												|
|  Adam Honse (CalcProgrammer1), 2010			|
\*---------------------------------------------*/
#include "serial.h"

#define SERIAL_BUFFER_LENGTH 16
unsigned char serial_buffer[SERIAL_BUFFER_LENGTH]; //Serial buffer
unsigned char serial_buffer_pos = 0; //Current use of buffer

//USART Receive interrupt pushes the incoming byte into the buffer
ISR(USART_RX_vect)
{
  serial_buffer[serial_buffer_pos] = UDR;
  
  if(++serial_buffer_pos > SERIAL_BUFFER_LENGTH)
  {
    serial_buffer_pos = 0;
  }
}

unsigned char serial_available()
{
  return serial_buffer_pos;
}

void serial_init(unsigned int baud)
{
	//Set baud rate
	UBRRH = (unsigned char) (baud >> 8);
	UBRRL = (unsigned char) (baud);

	//Set frame format: 8 data, no parity, 2 stop bits
	UCSRC = (0<<UMSEL) | (0<<UPM0) | (0<<USBS) | (3<<UCSZ0);

	//Enable receiver and transmitter
	UCSRB = (1<<RXCIE | 1<<RXEN) | (1<<TXEN);
}

//Buffered read pops the first byte off the buffer
unsigned char serial_read()
{
  if(serial_buffer_pos == 0)
  {
    return 0;
  }
  char value = serial_buffer[0];
  for(char i = 1; i < SERIAL_BUFFER_LENGTH; i++)
  {
    serial_buffer[i-1] = serial_buffer[i];
  }
  serial_buffer[15] = 0;
  serial_buffer_pos--;

  return value;
}

//Buffered read into user buffer
unsigned int serial_read_buffer(char* data, int len)
{
	int i = 0;
	for(; i < len; i++)
	{
		if(serial_available() > 0)
		{
			data[i] = serial_read();
		}
		else
		{
			return i;
		}
	}
	return i;
}

//Deletes all items in the buffer
void serial_flush()
{
  serial_buffer_pos = 0;
}

void serial_transmit_byte(char data)
{
	//Wait for empty transmit buffer
	while(!(UCSRA & (1<<UDRE)));

	//Put data into buffer, send data
	UDR = data;
}

void serial_transmit_data(char* buf, int len)
{
	for(int i = 0; i < len; i++)
	{
		serial_transmit_byte(buf[i]);
	}
}

void serial_print(char* text)
{
	int pos = 0;
	while(text[pos] != '\0')
	{
		serial_transmit_byte(text[pos]);
		pos++;
	}
}
