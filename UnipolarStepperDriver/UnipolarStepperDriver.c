/*-------------------------------------*\
| TinyStepper Unipolar Stepper Driver	|
| ATTiny2313-based unipolar driver for	|
| 6-wire or 8-wire stepper motors.		|
|										|
| Serial and i2c interface				|
| 										|
| Adam Honse (amhb59@mail.mst.edu/		|
|			  calcprogrammer1@gmail.com)|
| 3/31/2012								|
\*-------------------------------------*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include "serial.h"
#include "usi_i2c.h"

volatile signed char	step_state		= 0;
volatile char 			step_dir		= 1;
volatile char 			step_enabled	= 0;
volatile int			step_speed		= 0;
volatile int			step_count		= 0;

extern char usi_i2c_buffer[];
extern char usi_i2c_buffer_pos;
extern char usi_i2c_slave_address;

//Normal
//volatile char step_table[4] = { 0b00001000, 0b00100000, 0b00010000, 0b01000000};

//Half-stepping
//volatile char step_table[8] = { 0b00001000, 0b00101000, 0b00100000, 0b00110000, 0b00010000, 0b01010000, 0b01000000, 0b01001000};

//Double Phase
volatile char step_table[4] = { 0b00101000, 0b00110000, 0b01010000, 0b01001000};

void status_led_on();
void status_led_off();
void set_rgb_led(char led);
void initialize_timer();
void process_serial_message();
void process_i2c_message();
void fill_i2c_buffer_from_serial(char len, char addr, char rw);

void status_led_on()
{
	PORTD |= 0b00000100;
}

void status_led_off()
{
	PORTD &= 0b11111011;
}

void set_rgb_led(char led)
{
	led = led << 2;
	PORTB |= (led & 0b00011100);
	PORTB &= (led | 0b11100011);
}

void initialize_timer()
{
  OCR1A = 0; //Step Update value

  OCR0A = 32;
  OCR0B = 127;

  //Enable output compare A interrupt for timers 0 and 1
  TIMSK = (1<<OCIE1A | 1<<OCIE0A | 1<<OCIE0B);

  //Set frequency
  //CPU freq is 16MHz or 16,000,000 Hz
  //Timer freq is CPU/64 or 250,000 Hz
  //For 50Hz refresh rate, it should count to
  //250,000/50 which = 5000
  TCCR1B = (1<<CS10 | 1<<CS11 | 0<<CS12);

  TCCR0B = (0<<CS00 | 1<<CS01 | 0<<CS02);
}

ISR(TIMER1_COMPA_vect)
{
	if(step_enabled == 1)
	{
		if(step_dir == 1)
		{
			set_rgb_led(0b00000010);
  			if(++step_state > 3) step_state = 0;
		}
		else
		{
			set_rgb_led(0b00000001);
			if(--step_state < 0) step_state = 3;
		}

		if(step_count-- < 0) step_enabled = 0;
	}
	else
	{
		set_rgb_led(0b00000100);
	}

	TCNT1 = 0;
}

ISR(TIMER0_COMPB_vect)
{
	PORTD &= 0b10000111;
	TCNT0 = 0;
}

ISR(TIMER0_COMPA_vect)
{	
	if(step_enabled == 1)
	{
		PORTD |= step_table[step_state];
	}
}

int main()
{
	DDRB = 0b00011100;
	DDRD = 0b01111100;

	sei();
	initialize_timer();
	serial_init(65);

	usi_i2c_slave_address = eeprom_read_byte((uint8_t*)1);
	USI_I2C_Init(0, usi_i2c_slave_address);

	while(1)
	{
		process_serial_message();
		if(usi_i2c_buffer_pos > 3)
		{
			status_led_on();
			process_i2c_message();
			usi_i2c_buffer_pos = 0;
			status_led_off();
		}
	}
}

void process_serial_message()
{
	if(serial_available() > 2)
	{
		status_led_on();
		
		char buffer[3];
		serial_read_buffer(buffer, 3);

		switch(buffer[0])
		{
	/*		case 0x01:
				OCR1A = buffer[1] << 8 | buffer[2];
				break;

			case 0x02:
				step_count = buffer[1] << 8 | buffer[2];
				break;

			case 0x03:
				step_dir = buffer[1];
				break;

			case 0x04:
				step_enabled = buffer[1];
				break;
*/
			//Set I2C Mode (0 - slave, 1 - master)
			case 0x21:
				if(buffer[1] == 0)
				{
					//Slave Initialize
					USI_I2C_Init(0,usi_i2c_slave_address);
				}
				else
				{
					//Master Initialize
					USI_I2C_Init(1,0);
				}
				break;
			
			//Set I2C Address
			case 0x22:
				usi_i2c_slave_address = buffer[1];
				eeprom_write_byte((uint8_t*)1, usi_i2c_slave_address);
				break;

			//Read I2C Address
			case 0x23:
				serial_transmit_byte(usi_i2c_slave_address);
				break;

			//Send I2C Write
			case 0x24:
				fill_i2c_buffer_from_serial(buffer[1], buffer[2], 0);
				USI_I2C_Master_Transceiver_Start(usi_i2c_buffer, buffer[1]+1);
				process_i2c_message();
				break;

			//Send I2C Read
			case 0x25:
				{
				char addr = buffer[2] << 1 | 1;
				usi_i2c_buffer[0] = addr;
				USI_I2C_Master_Transceiver_Start(usi_i2c_buffer, buffer[1]+1);
				for(char i = 1; i <= buffer[1]; i++)
				{
					serial_transmit_byte(usi_i2c_buffer[i]);
				}
				}
				break;
		}
		status_led_off();	
	}
}

void process_i2c_message()
{
	if(usi_i2c_buffer[0] >> 1 == usi_i2c_slave_address || usi_i2c_buffer[0] >> 1 == 0)
	{
		switch(usi_i2c_buffer[1])
		{
			case 0x01:
				OCR1A = usi_i2c_buffer[2] << 8 | usi_i2c_buffer[3];
				break;

			case 0x02:
				step_count = usi_i2c_buffer[2] << 8 | usi_i2c_buffer[3];
				break;

			case 0x03:
				step_dir = usi_i2c_buffer[2];
				break;

			case 0x04:
				step_enabled = usi_i2c_buffer[2];
				break;
			
			case 0x05:
				OCR1A = usi_i2c_buffer[2];
				break;
		}
	}
}

void fill_i2c_buffer_from_serial(char len, char addr, char rw)
{
	//Set R/W bit of address
	addr = addr << 1 | rw;

	//Put address into i2c buffer
	usi_i2c_buffer[0] = addr;

	for(char i = 1; i <= len; i++)
	{
		while(serial_available() < 1);
		usi_i2c_buffer[i] = serial_read();
	}
}
