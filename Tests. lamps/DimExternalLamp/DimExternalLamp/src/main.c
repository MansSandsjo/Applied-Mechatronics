#define F_CPU 1000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


uint8_t cycle = 0;
unsigned char sensor1;
unsigned char sensor2;
int val = 0;
int AB = 0;

int main (void)
{
	
	DDRC &= ~((1<<PC5) | (1<<PC4)); //set pc5 and pc4 as input
	DDRD |= (1 << PD6);
	DDRB |= (1 << PB0);
	
	
	TCCR0A = (1 << COM0A1) | (1 << WGM00) | (1 << WGM01);

	
	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT13) |(1 << PCINT12);

	
	sei();
	
	TCCR0B |= (1 << CS00) | (1 << CS01);
	
	
	
	while(1)
	{
		OCR0A = cycle;
		_delay_ms(10);
	}
}


ISR(PCINT1_vect)
{
	
	sensor1 = PINC & (1 << PC5);
	sensor2 = PINC & (1 << PC4);
	val = sensor1 | sensor2;
	
	val = val >> PC4;
	
	switch (val){
		case 0 :
		if (AB==2)
		{
			cycle=cycle+10;
			
			PORTB|= (1 << PB0);
		}
		else if(AB==1)
		{
			cycle=cycle-10;
			PORTB &= ~(1<<PB0);
		}	}
	
	AB=val;
}
