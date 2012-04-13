/*---------------------------------*\
|  USI I2C/TWI Master - Adam Honse  |
|  Senior Design 3/18/2012	    |
\*---------------------------------*/

#include "usi_i2c_master.h"
#include <avr/io.h>
#include <util/delay.h>

void usi_i2c_master_initialize()
{
	//Enable pullups on SDA and SCL
	PORT_USI |= (1 << PIN_USI_SDA) | (1 << PIN_USI_SCL);

	//Set SDA and SCL as output
	DDR_USI &= ~((1 << PIN_USI_SDA) | (1 << PIN_USI_SCL));

	//Set up USI module//
	//Preload data register with "released level" data
	USIDR = 0xFF;
	
	//Disable interrupts, USI in Two-Wire mode, software strobe as counter clock source
	USICR = (0 << USISIE) | (0 << USIOIE) | (1 << USIWM1) | (0 << USIWM0) | (1 << USICS1) | (0 << USICS0) | (1 << USICLK) | (0 << USITC);

	//Clear flags and reset counter to zero
	USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) | (0x0 << USICNT0);
}

char usi_i2c_master_transfer(char data)
{
	//Load the USI data register with data to be transferred out
	USIDR = data;
	
	USISR = USISR_8_BIT;

	//Interrupts disabled, two wire mode, software clock strobe, toggle clock port
	char temp = (0<<USISIE)|(0<<USIOIE)| (1<<USIWM1)|(0<<USIWM0)| (1<<USICS1)|(0<<USICS0)|(1<<USICLK) | (1<<USITC);

	do
	{
		//Delay
		_delay_us(I2C_DELAY_2);

		//Generate positive SCL edge
		USICR = temp;

		//Wait for SCL to go high
		while(!(PIN_USI & (1 << PIN_USI_SCL)))
		{
		}

		_delay_us(I2C_DELAY_4);

		//Generate negative SCL edge		
		USICR = temp;

	}while(!(USISR & (1 << USIOIF)));

	_delay_us(I2C_DELAY_2);                

	//Read out data
	temp = USIDR;

	//Release SDA
	USIDR = 0xFF;

	//Enable SDA as output
	DDR_USI |= (1 << PIN_USI_SDA);

	//Return received data
	return temp;
}

int usi_i2c_master_transmit(char* data, int len)
{

	do

	{

		// Write a byte

		PORT_USI &= ~(1<<PIN_USI_SCL);					// Pull SCL LOW.

		USIDR     = *(data++);							// Setup data.

		usi_i2c_master_transfer(USISR_1_BIT);			// Send 8 bits on bus.



		// Clock and verify (N)ACK from slave

		DDR_USI  &= ~(1<<PIN_USI_SDA);					// Enable SDA as input.

	
		usi_i2c_master_transfer(USISR_1_BIT);


	}while(--len);										// Until all data sent/received.
	return 0;
}

int usi_i2c_master_receive(char* data, int len)
{

	do

	{

		// Read a data byte 

		DDR_USI &= ~(1<<PIN_USI_SDA);						// Enable SDA as input.


		*(data++) = usi_i2c_master_transfer(USISR_8_BIT);

		
		// Prepare to generate ACK (or NACK in case of End Of Transmission) 

		if(len == 1)									// If transmission of last byte was performed.

		{

			USIDR = 0xFF;									// Load NACK to confirm End Of Transmission.

		}

		else

		{

			USIDR = 0x00;									// Load ACK. Set data register bit 7 (output for SDA) low.

		}

		usi_i2c_master_transfer(USISR_1_BIT);				// Generate ACK/NACK.

	}while(--len);
	return 0;
}

int usi_i2c_master_write(char addr, char* data, int len)
{
	return 0;
}

int usi_i2c_master_read(char addr, char* data, int len)
{
	return 0;
}

void usi_i2c_master_stop()

{

	PORT_USI &= ~(1 << PIN_USI_SDA);			// Pull SDA low.

	PORT_USI |= (1 << PIN_USI_SCL);				// Release SCL.

	
	while(!(PIN_USI & (1 << PIN_USI_SCL)))		// Wait for SCL to go high.
	{
	}


	PORT_USI |= (1 << PIN_USI_SDA);				// Release SDA.

}

void usi_i2c_master_start()
{
	PORT_USI &= ~(1 << PIN_USI_SDA);			// Pull SDA low
	PORT_USI &= ~(1 << PIN_USI_SCL);			// Pull SCL low
}
