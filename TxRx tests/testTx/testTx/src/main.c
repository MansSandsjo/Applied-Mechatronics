

#include <avr/io.h>

#define F_CPU 1000000
#define BUAD 2400
#define BRC ((F_CPU/16/BUAD)-1)
#define TX_BUFFER_SIZE 128

#include <util/delay.h>

int main (void)
{
	DDRD  |= 1<<PIND1;
	UBRR0H = (BRC >> 8);
	UBRR0L = BRC;
	
	UCSR0B = (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	char printThis = 100;
	char printThis1 = printThis/100;
	char printThis2 = (printThis-printThis1*100);
	char printThis3 = (printThis-printThis1*100-printThis2*10);
	printThis1 +=48;
	printThis2 +=48;
	printThis3 +=48;
	
	while (1)
	{
		UDR0 = printThis1;
		UDR0 = printThis2;
		_delay_ms(10);
		UDR0 = printThis3;
		_delay_ms(1000);
	}

}
