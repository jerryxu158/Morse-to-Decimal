
#include <avr/interrupt.h>
volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){
	//avr timer/counter controller register tccr1
	TCCR1B = 0x0B;/*bit 3 = 0:ctc mode (clear timer on compare)
					bit2bit1bit0 = 011:pre-scaler /64
					0000 1011: 0x0B
					so 8MHz clock (8,000,000) / 64 = 125,000 ticks/second
					thus TCNT1 register will count at 125,000 ticks/s*/
					
	//AVR output compare register OCR1A
	OCR1A = 125; /* timer interrupt will be generated when TCNT1 == OR1A
				we want 1 ms tick. 0.001 s * 125,000 ticks/s = 125
				so when TCNT1 register equals 125,*/
	
	//avr timer interrupt mask reg			
	TIMSK1 = 0x02;/* bit1: OCIE1A -- enables compare match interrupt*/
	
	//initialize avr counter
	TCNT1 = 0;
	
	_avr_timer_cntcurr = _avr_timer_M;
	//timerisr will be called every _avr_timer_cntcurr milliseconds
	
	//enable global interrupts
	SREG |= 0x80;	
}

void TimerOff(){
	TCCR1B = 0x00;
}

void TimerISR(){
		TimerFlag = 1;
}
//in our approach
ISR(TIMER1_COMPA_vect){
	//cpu auto calls when TCNT1 == OCR1 (ever 1 ms per TimerOn settings)
	_avr_timer_cntcurr--;//coutn down to 0 rather than up to TOP
	if(_avr_timer_cntcurr == 0){
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}




