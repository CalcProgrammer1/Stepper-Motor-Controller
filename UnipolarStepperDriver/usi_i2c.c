/*-----------------------------------------------------*\
|  USI I2C Master/Slave Driver - Adam Honse 2012	|
\*-----------------------------------------------------*/

#include "usi_i2c.h"

#define temp_USISR_8bit (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)| (0x00<<USICNT0)
#define temp_USISR_1bit (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)| (0x0E<<USICNT0)

#define USI_I2C_BUFFER_LENGTH 16
char usi_i2c_buffer[USI_I2C_BUFFER_LENGTH];
char usi_i2c_buffer_pos = 0;
char usi_i2c_slave_address;
char usi_i2c_mode;

struct USI_I2C_Master_State
{
	char address_mode		: 1;
	char write_mode			: 1;
	char error_state		: 6;
} USI_I2C_Master_State;

typedef enum
{
  USI_SLAVE_CHECK_ADDRESS                = 0x00,
  USI_SLAVE_SEND_DATA                    = 0x01,
  USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA = 0x02,
  USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA   = 0x03,
  USI_SLAVE_REQUEST_DATA                 = 0x04,
  USI_SLAVE_GET_DATA_AND_SEND_ACK        = 0x05
} USI_I2C_Slave_State_t;

USI_I2C_Slave_State_t USI_I2C_Slave_State;

//USI I2C Initialize
//  1. master - Set to 0 for slave, set to 1 for master
//  2. address - If slave, this parameter is the slave address
void USI_I2C_Init(char master, char address)
{
	DDR_USI  |= (1 << PORT_USI_SDA) | (1 << PORT_USI_SCL);
	PORT_USI |= (1 << PORT_USI_SCL);
	PORT_USI |= (1 << PORT_USI_SDA);
	

	if(master == 0)
	{
		usi_i2c_slave_address = address;
		DDR_USI &= ~(1 << PORT_USI_SDA);
		//DDR_USI &= ~(1 << PORT_USI_SCL);
		USICR = (1 << USISIE) | (0 << USIOIE) | (1 << USIWM1) | (0 << USIWM0) | (1 << USICS1) | (0 << USICS0) | (0 << USICLK) | (0 << USITC);
		USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC);
	}
	else
	{
		USIDR = 0xFF;
		USICR = (0 << USISIE) | (0 << USIOIE) | (1 << USIWM1) | (0 << USIWM0) | (1 << USICS1) | (0 << USICS0) | (1 << USICLK) | (0 << USITC);
		USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF)  | (1 << USIDC)  | (0x00 << USICNT0);
	}
}

char USI_I2C_Master_Transfer(char temp)
{
	USISR = temp;				//Set USISR as requested by calling function
	temp = (0<<USISIE) | (0<<USIOIE) | (1<<USIWM1) | (0<<USIWM0) | (1<<USICS1) | (0<<USICS0) | (1<<USICLK) | (1<<USITC);

	// Shift Data
	do
	{
		_delay_us (I2C_TLOW);
		USICR = temp;				//SCL Positive Edge
		while (!(PIN_USI&(1<<PIN_USI_SCL)));	//Wait for SCL to go high
		_delay_us (I2C_THIGH);
		USICR = temp;				//SCL Negative Edge
	} while (!(USISR&(1<<USIOIF)));			//Do until transfer is complete
	
	_delay_us (I2C_TLOW);
	temp = USIDR;
	USIDR = 0xFF;
	DDR_USI |= (1<<PORT_USI_SDA);			//Set SDA as output			

	return temp;					//Return data from USIDR
}

char USI_I2C_Master_Transceiver_Start(char *msg, char msg_size)
{
	USI_I2C_Master_State.address_mode = 1;

	if(!(*msg & 0x01)) // Check if message is read/write
	{
		USI_I2C_Master_State.write_mode = 1;
	}
	else
	{
		USI_I2C_Master_State.write_mode = 0;
	}
	
	PORT_USI |= (1<<PORT_USI_SCL);		//Pull SCL High
	
	while (!(PIN_USI & (1<<PIN_USI_SCL)));	//Wait for SCL to go high

	#ifdef I2C_FAST_MODE
		_delay_us (I2C_THIGH);
	#else
		_delay_us (I2C_TLOW);
	#endif

	//Send Start Condition
	PORT_USI &= ~(1<<PORT_USI_SDA);		//Pull SDA low
	_delay_us (I2C_THIGH);
	PORT_USI &= ~(1<<PORT_USI_SCL);		//Pull SCL low
	_delay_us (I2C_TLOW);
	PORT_USI |= (1<<PORT_USI_SDA);		//Pull SDA high
	
	do
	{
		if(USI_I2C_Master_State.address_mode || USI_I2C_Master_State.write_mode)
		{
			PORT_USI &= ~(1<<PORT_USI_SCL);		//Pull SCL low
			USIDR = *(msg++);			// Load data then increment buffer
			USI_I2C_Master_Transfer(temp_USISR_8bit);

			//Get Acknowledgement from slave
			DDR_USI &= ~(1<<PORT_USI_SDA);		//Set SDA as input
			if(USI_I2C_Master_Transfer (temp_USISR_1bit) & 0x01)
			{
				return 0;
			}
			USI_I2C_Master_State.address_mode = 0;
		}

		else
		{
			DDR_USI &= ~(1<<PORT_USI_SDA);		//Set SDA as input
			*(msg++) = USI_I2C_Master_Transfer(temp_USISR_8bit);
			
			// Read data then increment buffer
			if(msg_size == 1)			//Last byte transmitted
			{
				USIDR = 0xFF;			//Load NACK to end transmission
			}
			else
			{
				USIDR = 0x00;			//Load ACK
			}
			USI_I2C_Master_Transfer(temp_USISR_1bit);	//Output Acknowledgement
		}
	}while(--msg_size);			//Do until all data is read/written

	PORT_USI &= ~(1<<PIN_USI_SDA);           // Pull SDA low.
	PORT_USI |= (1<<PIN_USI_SCL);            // Release SCL.
	while( !(PIN_USI & (1<<PIN_USI_SCL)) );  // Wait for SCL to go high.  
	_delay_us(I2C_THIGH);
	PORT_USI |= (1<<PIN_USI_SDA);            // Release SDA.
	_delay_us(I2C_TLOW);

	return 1;
}

ISR(USI_START_vect)
{
	// set default starting conditions for new TWI package
	USI_I2C_Slave_State = USI_SLAVE_CHECK_ADDRESS;
	
	usi_i2c_buffer_pos = 0;
	// set SDA as input
	DDR_USI &= ~( 1 << PORT_USI_SDA );

	// wait for SCL to go low to ensure the Start Condition has completed (the
	// start detector will hold SCL low ) - if a Stop Condition arises then leave
	// the interrupt to prevent waiting forever - don't use USISR to test for Stop
	// Condition as in Application Note AVR312 because the Stop Condition Flag is
	// going to be set from the last TWI sequence
	while((PIN_USI & (1 << PIN_USI_SCL)) && !((PIN_USI & (1 << PIN_USI_SDA))));

	if(!(PIN_USI & (1 << PIN_USI_SDA)))
	{
		// a Stop Condition did not occur
		USICR = (1 << USISIE) | (1 << USIOIE) | (1 << USIWM1) | (1 << USIWM0) | (1 << USICS1) | (0 << USICS0) | (0 << USICLK) | (0 << USITC);
	}
	else
	{
		// a Stop Condition did occur
    	USICR = (1 << USISIE) | (0 << USIOIE) | (1 << USIWM1) | (0 << USIWM0) | (1 << USICS1) | (0 << USICS0) | (0 << USICLK) | (0 << USITC);
	}

	USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) |(1 << USIDC) | (0x0 << USICNT0);
}

ISR(USI_OVERFLOW_vect)
{
	switch (USI_I2C_Slave_State)
	{
		case USI_SLAVE_CHECK_ADDRESS:
			if((USIDR == 0) || ((USIDR >> 1) == usi_i2c_slave_address))
			{
				usi_i2c_buffer[usi_i2c_buffer_pos] = USIDR;
				usi_i2c_buffer_pos++;
				if (USIDR & 0x01)
				{
					USI_I2C_Slave_State = USI_SLAVE_SEND_DATA;
				}
				else
				{
					USI_I2C_Slave_State = USI_SLAVE_REQUEST_DATA;
				}

				//Set USI to send ACK
				USIDR = 0;
				DDR_USI |= ( 1 << PORT_USI_SDA );
				USISR = (0 << USISIF) | (1 << USIOIF) | (1 << USIPF)  | (1 << USIDC)  | (0x0E << USICNT0);
			}
			else
			{
				//Set USI to Start Condition Mode
				USICR = (1 << USISIE) | (0 << USIOIE) | (1 << USIWM1) | (0 << USIWM0) | (1 << USICS1) | (0 << USICS0) | (0 << USICLK) | (0 << USITC);
				USISR = (0 << USISIF) | (1 << USIOIF) | (1 << USIPF)  | (1 << USIDC)  | (0x0 << USICNT0);
			}
			break;

		case USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA:
			if(USIDR)
			{
				//If NACK, the master does not want more data
				//Set USI to Start Condition Mode
				USICR = (1 << USISIE) | (0 << USIOIE) | (1 << USIWM1) | (0 << USIWM0) | (1 << USICS1) | (0 << USICS0) | (0 << USICLK) | (0 << USITC);
				USISR = (0 << USISIF) | (1 << USIOIF) | (1 << USIPF)  | (1 << USIDC)  | (0x0 << USICNT0);
				return;
			}

		case USI_SLAVE_SEND_DATA:
			// Get data from Buffer
//			if ( txHead != txTail )
//			{
//				txTail = ( txTail + 1 ) & TWI_TX_BUFFER_MASK;
//				USIDR = usi_i2c_buffer[ txTail ];
//			}
//			else
			{
				//The buffer is empty
				//Set USI to Start Condition Mode
				USICR = (1 << USISIE) | (0 << USIOIE) | (1 << USIWM1) | (0 << USIWM0) | (1 << USICS1) | (0 << USICS0) | (0 << USICLK) | (0 << USITC);
				USISR = (0 << USISIF) | (1 << USIOIF) | (1 << USIPF)  | (1 << USIDC)  | (0x0 << USICNT0);
				return;
			}
			USI_I2C_Slave_State = USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA;

			//Set USI to send data
			DDR_USI |= (1 << PORT_USI_SDA);
			USISR    = (0 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) | (0x0 << USICNT0);
			break;

		case USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA:
			USI_I2C_Slave_State = USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA;
			
			//Set USI to read ACK
			DDR_USI &= ~(1 << PORT_USI_SDA);
			USIDR = 0;
			USISR = (0 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) | (0x0E << USICNT0);
			break;

		case USI_SLAVE_REQUEST_DATA:
			USI_I2C_Slave_State = USI_SLAVE_GET_DATA_AND_SEND_ACK;

			//Set USI to read data
			DDR_USI &= ~( 1 << PORT_USI_SDA );
			USISR    = (0 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) | (0x0 << USICNT0);
			break;

		case USI_SLAVE_GET_DATA_AND_SEND_ACK:
			//Put data into buffer
			//Not necessary, but prevents warnings
			usi_i2c_buffer[usi_i2c_buffer_pos] = USIDR;
			usi_i2c_buffer_pos++;
//			rxHead = ( rxHead + 1 ) & TWI_RX_BUFFER_MASK;
//			usi_i2c_buffer[ rxHead ] = USIDR;
			
			USI_I2C_Slave_State = USI_SLAVE_REQUEST_DATA;

			//Set USI to send ACK
			USIDR = 0;
			DDR_USI |= ( 1 << PORT_USI_SDA );
			USISR = (0 << USISIF) | (1 << USIOIF) | (1 << USIPF)  | (1 << USIDC)  | (0x0E << USICNT0);
			break;
	}
}
