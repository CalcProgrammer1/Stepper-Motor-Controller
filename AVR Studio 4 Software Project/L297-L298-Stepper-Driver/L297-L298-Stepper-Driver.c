/*-------------------------------------------------------------*\
|  L297 Stepper Motor Controller with Serial and I2C Interface	|
|  Based on ATTiny2313 microcontroller, this motor controller	|
|  can drive stepper motors using the ST L297/298 chipset		|
|																|
|  Adam Honse (amhb59@mail.mst.edu, calcprogrammer1@gmail.com)	|
|  3/16/2012, Designed for Senior Design Project				|
\*-------------------------------------------------------------*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "serial.h"
#include "usi_i2c_master.h"

//L297 Pins on Port B
#define L297_PORTB_DDR			0b00011111
#define L297_PIN_HF				PB0
#define L297_PIN_CW_CCW			PB1
#define L297_PIN_CLOCK			PB2
#define L297_PIN_SYNC			PB3
#define L297_PIN_ENABLE			PB4

//L297 Pins on Port D
#define L297_PORTD_DDR			0b01100000
#define L297_PIN_HOME			PD2
#define L297_PIN_CNTL			PD5
#define L297_PIN_RESET			PD6

//Set up the timer overflow to generate a clock for the L297's SYNC line
void setup_l297_clock()
{
   DDRB = L297_PORTB_DDR;
   DDRD = L297_PORTD_DDR;

   OCR1A = 0;
   TCCR1A  = ((1 << COM1A1) | (1 << WGM10) | (1 << WGM11));
   TCCR1B  = ((1 << CS10) | (0 << CS11) | (1 << CS12) | (0 << WGM12));
   TIMSK  = (0<<OCIE1B);

   OCR1A = 255;
}


int main()
{
	//Initialize serial interface to 38400 baud
	serial_init(12);

	//Start SYNC clock for L297
	setup_l297_clock();

	//Start USI I2C Interface
	usi_i2c_master_initialize();

	//Enable L297
	PORTB |= (1 << L297_PIN_ENABLE);

	while(1)
	{
		_delay_ms(1000);
		usi_i2c_master_stop();
	}
}
