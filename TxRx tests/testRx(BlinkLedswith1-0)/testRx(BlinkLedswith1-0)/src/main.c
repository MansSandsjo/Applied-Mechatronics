
#include <asf.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#define F_CPU 1000000
#define BUAD 2400
#define BRC	((F_CPU/16/BUAD)-1)
#define RX_BUFFER_SIZE 128

char rxBuffer[RX_BUFFER_SIZE];
uint8_t rxReadPos=0;
uint8_t rxWritePos=0;

char getChar(void);
char peekChar(void);
unsigned char USART_Receive( void );

int main (void)
{
	UBRR0H = (BRC >> 8);
	UBRR0L = BRC;
	
	UCSR0B = (1 << RXEN0)|(1<<TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	
	DDRB = (1 << PB0) | (1 << PB7);
	DDRD = (1 << PD7);
	sei();

	
	while(1)
	{
		char c = USART_Receive();
		if (c == '1')
		{
			sbi(PORTB, PORTB0);
			sbi(PORTB, PORTB7);
			sbi(PORTD, PORTD7);
		}
		else if (c == '0')
		{
			cbi(PORTB, PORTB0);
			cbi(PORTB, PORTB7);
			cbi(PORTD, PORTD7);
		}
		
	}
	
	
	
}

char peekChar()
{
	char r = '\0';
	
	if(rxReadPos != rxWritePos)
	{
		r = rxBuffer[rxReadPos];
	}
	return r;
}

unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
	;
	/* Get and return received data from buffer */
	return UDR0;
}

char getChar()
{
	char r = '0';
	
	if(rxReadPos != rxWritePos)
	{
		r = rxBuffer[rxReadPos];
		r++;
	}
	else if(rxReadPos >= RX_BUFFER_SIZE)
	{
		rxReadPos = 0;
	}
	return r;
}

ISR(USART_RX_vect)
{
	rxBuffer[rxWritePos] = UDR0;
	rxWritePos++;
	
	if(rxWritePos >= RX_BUFFER_SIZE)
	{
		rxWritePos = 0;
	}
}