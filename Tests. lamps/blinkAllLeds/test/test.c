/*
 * test.c
 *
 * Created: 2020-11-13 16:53:17
 *  Author: tmk20ms
 */ 

#define F_CPU 1000000
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	
	
	while(1)
    {
		DDRB |= 1<<PINB0;
		DDRB |= 1<<PINB7;
		DDRD |= 1<<PIND7;
		PORTD|= 1<<PIND7;
		PORTB|= 1<<PINB0;
		PORTB|= 1<<PINB7;
		_delay_ms(200);
		DDRB &= 0b00000000;
		PORTB&= 0b00000000;
		DDRD &= 0b00000000;
		PORTD&= 0b00000000;
		_delay_ms(200);
    }
}