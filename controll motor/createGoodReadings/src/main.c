
#include <asf.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define F_CPU 1000000
#define BUAD 2400
#define BRC	((F_CPU/16/BUAD)-1)
#define RX_BUFFER_SIZE 128

#define upLimit 15625 //15 625 ticks per revolution at 5rpm
#define lowLimit 200 //651 ticks per revolution at 120rpm
#define size_rpm 16;
#include <util/delay.h>

//char rxBuffer[RX_BUFFER_SIZE];
//uint8_t rxReadPos=0;
//uint8_t rxWritePos=0;

unsigned int counter;
unsigned int cycle = 0;

//-----read signal from motor & Tx variables-----
char rec_rpm[8];
unsigned int time;
unsigned int avg_ticks;
volatile unsigned int ticks_time;
volatile unsigned int medianVect[5];
unsigned int indexMedian = 0;
char charBuffer[5];
volatile unsigned int send_median;
unsigned int r = 0;
int receive_vect [4];
unsigned char receive;
char charFunc;


//------regulator variables------
int integral_error;
volatile int setRpm;
int rpmToMotor;
int error;
#define kp 2 //double kp = 0.25;
#define ki 2 //double ki = 0.25;
#define frac 3
int fine;


unsigned char USART_Receive( void );
void USART_Transmit( unsigned char data );
unsigned int TIM16_ReadTCNT1( void );
void TIM16_WriteTCNT1( unsigned int i );
void calcMedian(unsigned int time);

void Array_sort(volatile unsigned int *array , int n);
void controllMotor(int error);
void setUpTimerforRegulator();
void startADCconversion();
// denna måste vara den jag använde, yes här är PI regulator o så
int main (void)
{
	UBRR0H = (BRC >> 8);
	UBRR0L = BRC;
	DDRD |= (1 << PD6) | (1<<PIND1) | (1 << PD7);
	DDRC &= ~((1<<PC5) | (1<<PC4));

	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT13) | (1 << PCINT12);
	
	/*----------trimmer/finetuner--------*/
	//disable 
	
	ADMUX = (1<<REFS0) | (1 << MUX1);
	ADCSRA = (1 << ADEN) | (1 << ADIE); 
	DIDR0 = (1 << ADC2D);
	/*-----------------------------------*/
	 
	UCSR0B = (1 << RXEN0)  | (1<<TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	
	TCCR0A = (1 << COM0A1) | (1 << WGM00) | (1 << WGM01) | (1<<COM0A0) | (1 << CS00);
	TCCR0B |= (1 << CS00); // (1 << CS01);
	OCR0A = 0;
	OCR0B = 0;
	
	
	setUpTimerforRegulator();
	
	sei();
	//timer 16bit counter with prescaler 8
	TCCR1B |= (1 << CS11);
	//timer for regulator
	TCCR2B = (1 << CS21) | (1 << CS22) | (1 << CS20);
	
	
	
	
	while(1)
	{
		for(int r = 0; r < 3; r++){
			char received = USART_Receive();
			//save to controll the AVR with different char
			if(r==0){
				charFunc = received;
			}
			receive_vect[r] = received - '0';
		}
		//blink LED on AVR for confirmation
		
		
		int input = receive_vect[0]*100+receive_vect[1]*10+receive_vect[2];
		
		//controller values to set and receive signal from AVR
		if(input <= 120 && input >= 10){
			integral_error = 0;
			setRpm = input;
			}
			//transmit RPM if 's00' is sent
		else if (charFunc == 's'){
				PORTD ^= (1 << PD7);
				itoa(send_median,charBuffer,10);
				for (int i = 0; i < 5; i++){
					USART_Transmit(charBuffer[i]);
				}
				USART_Transmit(' ');
		}
				
			//finjustera rpm
		/*else if (charFunc == 'u'){
			setRpm++;
		}
		
		else if (charFunc == 'n'){
			setRpm--;
		}*/
	_delay_ms(50); // works better to Rx with this delay
	}
	
}

void controllMotor(int error){
	 
	//error = setRpm-currentValue+fineTunerKnob;
	integral_error += error;
	
				//	P			  I
	int temp= (kp*error) + ki*integral_error;
	temp=(temp>>(frac)); // genom 2^3
		rpmToMotor = temp;
		
	if(rpmToMotor > 255){
		rpmToMotor = 255;
	}else  if(rpmToMotor < 0){
		rpmToMotor = 0 ;
	}
	OCR0A = rpmToMotor;
}
unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)))
	;
	/* Get and return received data from buffer */
	return UDR0;
}


void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

// läs rpm från motor digital encoder
ISR(PCINT1_vect)
{
	time = TIM16_ReadTCNT1();
	TIM16_WriteTCNT1(0);
	calcMedian(time);
	
}
ISR(ADC_vect){
	
}

void calcMedian(unsigned int time){
	
	//filter signals
	if((lowLimit < time) && (time < upLimit)){
		ticks_time+=time;
		cycle++;
		
		if(cycle==4){
			cycle=0;
				avg_ticks= (ticks_time>>2);
			ticks_time=0;
		//now save average of the ticks
		medianVect[indexMedian]=avg_ticks;
		indexMedian++;
		//when the vector is full Tx the median value
		if(indexMedian==5)
		{
			
			Array_sort(medianVect,5);
			send_median = 78125/(medianVect[2]);
			indexMedian=0;
			}
		}
	}
}
ISR(TIMER2_COMPB_vect){

	/* fine tune*/
	fine = (((ADC>>6)*10)>>3)-9;
	startADCconversion();
	controllMotor(setRpm-send_median+fine);
}

unsigned int TIM16_ReadTCNT1( void )
{
	unsigned char sreg;
	unsigned int i;
	/* Save global interrupt flag */
	sreg = SREG;
	/* Disable interrupts */
	cli();
	/* Read TCNT1 into i */
	i = TCNT1;
	/* Restore global interrupt flag */
	SREG = sreg;
	return i;
}

void TIM16_WriteTCNT1( unsigned int i )
{
	unsigned char sreg;
	//unsigned int i;
	/* Save global interrupt flag */
	sreg = SREG;
	/* Disable interrupts */
	cli();
	/* Set TCNT1 to i */
	TCNT1 = i;
	/* Restore global interrupt flag */
	SREG = sreg;
}


void Array_sort(volatile unsigned int *array , int n){
	// declare some local variables
	int i=0 , j=0 , temp=0;

	for(i=0 ; i<n ; i++)
	{
		for(j=0 ; j<n-1 ; j++)
		{
			if(array[j]>array[j+1])
			{
				temp        = array[j];
				array[j]    = array[j+1];
				array[j+1]  = temp;
			}
		}
	}
}

void setUpTimerforRegulator(){
	/* calculations from https://eleccelerator.com/avr-timer-calculator/ */
	TCCR2B = (1 << WGM21);
	TIMSK2 |= (1 << OCIE2B);
	OCR2B   = 244; //trigger ISR 4 times per s (4Hz)	
}

void startADCconversion(){
	
	ADCSRA |= (1 << ADSC);
}

/*börja närmast datorn och jobba mot motorn
0. Lukta och kolla på brädorna
1. funkar kabeln? kortslut och loopback kabeln till motorn, funkar det att skicka något? Får du en etta 
när du skickar något från Hyperterminal
2. När serial funkar koppla på opto och försök få den att funka med loopback, fortsätt med invertern
3. Testa AVR:n för sig. Funkar det att lysa lampor ? PWM signalen? (Går det att generera en PWM signal med encodern?)  Skicka och ta mot från dator? ADCn?
*/
