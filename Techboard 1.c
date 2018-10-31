/*
 * Techboard_1.c
 *
 * Created: 25.08.2013 11:17:35
 *  Author: Florian
 * History: 25.08.2013: Created Mainstructure
 *			16.01.2014: Removed unnecessary Functions
 */ 

// *** Improvements ***

// Stopwatch
// Pulsen der Helligkeit via Segmentpins


// *** Definitions ***

#define F_CPU		4000000UL
#define FREQ		4000000
#define OFF			10
#define FULL		11
#define E			12
#define r			13


// *** Includes ***

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


// *** Volatiles ***

volatile uint8_t  output_value_1, output_value_2, dotpoint = 0, next_display = 1;


// *** Prototypes Generic ***

void segmentdisplay_output(int);
void segmentdisplay_multiplex_on(void);
void segmentdisplay_multiplex_off(void);
void segmentdisplay_boot(void);
void segmentdisplay_error(void);
void segmentdisplay_integersplit(int);
void segmentdisplay_floatsplit(float);
void segmentdisplay_dotpoint_on(void);
void segmentdisplay_dotpoint_off(void);


// *** Prototypes Main ***

void init_ports(void);
void counter (void);
void multiplex_via_polling(void);
void multiplex_via_interrupt(void);
void counter99_via_interrupt(void);
void counter9dot9_via_interrupt(void);
void error_test(void);
void integersplit_test(void);
void floatsplit_test(void);
void select_integer_or_float(void);


// *** Hauptprogramm ***

int main(void)
{
	init_ports();
	segmentdisplay_boot();
	//counter();
	//multiplex_via_polling();
	//multiplex_via_interrupt();
	//counter99_via_interrupt();
	//counter9dot9_via_interrupt();
	//error_test();
	//integersplit_test();
	//floatsplit_test();
	select_integer_or_float();
}


// *** ISR Functions ***

ISR (TIMER1_COMPA_vect)
{
	
	// 26.08.2013: Developed new structure
	
	// 27.08.2013: Ghost Effect removed by optimized structure
	
	switch (next_display)
	{
		case 1:			PORTC=0xFF;
						next_display = 2;
						break;
		case 2:			segmentdisplay_output(output_value_1);
						if(dotpoint==1)
						{
							PORTD &= 0b01111111;			// Dotpoint ON
						}
						else
						{
							PORTD |= 0b10000000;			// Dotpoint OFF
						}
						next_display = 3;
						break;
		case 3:			PORTC=0x01;				// Display 1
						next_display = 4;
						break;
		case 4:			PORTC=0xFF;
						next_display = 5;
						break;
		case 5:			segmentdisplay_output(output_value_2);
						next_display = 6;
						break;
		case 6:			PORTC=0x02;				// Display 2
						next_display = 1;
						break;
	}
}


// *** Generic Functions ***

void segmentdisplay_output(int number)
{	
	switch(number)
	{
		case 0:		PORTD = 0b11000000;
		break;
		case 1:		PORTD = 0b11111001;
		break;
		case 2:		PORTD = 0b10100100;
		break;
		case 3:		PORTD = 0b10110000;
		break;
		case 4:		PORTD = 0b10011001;
		break;
		case 5:		PORTD = 0b10010010;
		break;
		case 6:		PORTD = 0b10000010;
		break;
		case 7:		PORTD = 0b11111000;
		break;
		case 8:		PORTD = 0b10000000;
		break;
		case 9:		PORTD = 0b10010000;
		break;
		case 10:	PORTD = 0b11111111;				// OFF
		break;
		case 11:	PORTD = 0b00000000;				// FULL
		break;
		case 12:	PORTD = 0b10000110;				// "E"
		break;
		case 13:	PORTD = 0b10101111;				// "r"
		break;
	}
}


void segmentdisplay_multiplex_on(void)
{
	TCCR1B = (1<<WGM12) | (1<<CS10);
	TIMSK = (1<<OCIE1A);
	OCR1A = FREQ/4000;							// Entspricht 4k Hz (Optimale u. schnellste Multiplex Frequenz)
	sei();
}


void segmentdisplay_multiplex_off(void)
{
	cli();
}


void segmentdisplay_boot(void)
{
	int i;
	
	PORTC=0x00;
	for(i=1;i<=3;i++)
	{
		PORTD=0b11111110;
		_delay_ms(100);
		PORTD=0b11111101;
		_delay_ms(100);
		PORTD=0b11111011;
		_delay_ms(100);
		PORTD=0b11110111;
		_delay_ms(100);
		PORTD=0b11101111;
		_delay_ms(100);
		PORTD=0b11011111;
		_delay_ms(100);
	}
	segmentdisplay_output(OFF);
	_delay_ms(500);
	segmentdisplay_output(FULL);
	_delay_ms(300);
	segmentdisplay_output(OFF);
	_delay_ms(300);
	segmentdisplay_output(FULL);
	_delay_ms(300);
	segmentdisplay_output(OFF);
	PORTC=0xFF;
	_delay_ms(500);
}


void segmentdisplay_error(void)
{
		output_value_1 = E;
		output_value_2 = r;
}


void segmentdisplay_integersplit(int value_1)
{
	uint8_t ones = 0, tens = 0;
	
	if(value_1<0 || value_1>99)
	{
		segmentdisplay_error();
	}
	else
	{
		while(value_1>10)
		{
			value_1-=10;
			tens++;
		}
		ones = value_1;
		output_value_2 = ones;
		if (tens)
		{
			output_value_1 = tens;
		}
		else
		{
			output_value_1 = OFF;
		}
	}
}


void segmentdisplay_floatsplit(float value_1)
{
	value_1*=10;
	if(value_1<0 || value_1>99)
	{
		segmentdisplay_error();
	}	
	else
	{
		segmentdisplay_dotpoint_on();
		segmentdisplay_integersplit(value_1);
	}	
}


void segmentdisplay_dotpoint_on(void)
{
	dotpoint = 1;
}

void segmentdisplay_dotpoint_off(void)
{
	dotpoint = 0;
}


// *** Main Functions ***

void init_ports (void)
{
	DDRD = 0xFF;
	DDRC = 0xFF;
	PORTD = 0xFF;
	PORTC = 0xFF;
}


void counter(void)
{
	uint8_t i=0;
	
	segmentdisplay_multiplex_off();	
	PORTC=0x00;
	while(1)
	{
		for(i=0;i<=9;i++)
		{
			segmentdisplay_output(i);
			_delay_ms(1000);
		}
	}
}


void multiplex_via_polling(void)
{
	while(1)
	{
		PORTC = 1<<PINC0;
		segmentdisplay_output(1);
		_delay_ms(5);
		PORTC = 1<<PINC1;
		segmentdisplay_output(2);
		_delay_ms(5);
	}
}


void multiplex_via_interrupt(void)
{
	uint8_t i;
	
	
	segmentdisplay_multiplex_on();
	while(1)
	{
		for(i=1; i<=9; i++)
		{
			output_value_1 = i;
			output_value_2 =i-1;
			_delay_ms(1000);
		}
	}
}


void counter99_via_interrupt(void)
{
	uint8_t i, j;

	segmentdisplay_multiplex_on();
	while(1)
	{
		for(i=0;i<=9;i++)
		{
			output_value_1 = i;
			for(j=0;j<=9;j++)
			{
				output_value_2 = j;
				_delay_ms(1000);
			}
		}
	}	
}


void counter9dot9_via_interrupt(void)
{
	uint8_t i, j;
	
	segmentdisplay_multiplex_on();
	segmentdisplay_dotpoint_on();
	while (1)
	{
		for (i=0;i<=9;i++)
		{
			output_value_1 = i;
			for(j=0;j<=9;j++)
			{
				output_value_2 = j;
				_delay_ms(100);
			}
		}
	}	
}


void error_test(void)
{
	segmentdisplay_multiplex_on();
	while (1)
	{
		segmentdisplay_error();
	}
}


void integersplit_test(void)
{
	segmentdisplay_multiplex_on();
	while(1)
	{
		segmentdisplay_integersplit(69);
	}	
}


void floatsplit_test(void)
{
	segmentdisplay_multiplex_on();
	while(1)
	{
		segmentdisplay_floatsplit(2.9);
	}	
}

void select_integer_or_float(void)
{
	float temp, example_value = 69;			// Variable kann auch Ganzzahl "annehmen"...
	
	segmentdisplay_multiplex_on();
	while(1)
	{
		temp = example_value;
		while(temp>=1)
		{
			temp--;
		}
		if(temp==0)
		{
			segmentdisplay_integersplit(example_value);
		}
		else
		{
			segmentdisplay_floatsplit(example_value);
		}		
	}	
}