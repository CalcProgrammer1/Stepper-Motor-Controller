/*---------------------------------*\
|  USI I2C/TWI Master - Adam Honse  |
|  Senior Design 3/18/2012	    |
\*---------------------------------*/

//Port registers for the USI pins
#define PORT_USI 	PORTB
#define PIN_USI		PINB
#define DDR_USI 	DDRB

//Pin bit positions for SDA and SCL
#define PIN_USI_SDA 5
#define PIN_USI_SCL 7

//Delays, use with _delay_us()
#define I2C_DELAY_2 5	// > 4.7 uS
#define I2C_DELAY_4 4	// > 4.0 uS

#define USISR_8_BIT	0b11110000
#define USISR_1_BIT	0b11111110

void usi_i2c_master_initialize();

int usi_i2c_master_transmit(char* data, int len);

int usi_i2c_master_receive(char* data, int len);

int usi_i2c_master_write(char addr, char* data, int len);

int usi_i2c_master_read(char addr, char* data, int len);

void usi_i2c_master_stop();
