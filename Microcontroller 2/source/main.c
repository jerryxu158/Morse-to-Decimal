#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include <timer.h>
#endif

enum rStates{rInit, rWait, receive, rBuf} rState; unsigned char input = 0, rCount = 5;
enum combine{cInit, combine} cState;
int rTick(int);
int combineTick(int); unsigned char outputRec;

typedef struct task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;
task tasks[2];
unsigned char tasksNum = 2;

void initArr();

int main(void) {
	DDRC = 0xFF; PORTC = 0x00;
	DDRB = 0xFF; PORTB = 0xFF;
	DDRD = 0x00; PORTD = 0xFF;
    initArr();
    TimerOn();
  	TimerSet(100);
  	nokia_lcd_init();
  	
  	unsigned char tasksPeriodGCD = 100;
    while (1) {
		for (int i=0; i < tasksNum; i++) {
			PINA = PINA & 0x0F;
    	    if (tasks[i].elapsedTime >= tasks[i].period){
		    tasks[i].state = tasks[i].TickFct(tasks[i].state); 
		    tasks[i].elapsedTime = 0;
	    	}
	    tasks[i].elapsedTime += tasksPeriodGCD;
	   	}
	   	while(!TimerFlag);
	   	TimerFlag = 0;
	}
    
    return 1;
}

int rTick(int state){
	switch(state){//transitions
		case rInit:
			state = rWait;
			input = 0;
			outputRec = 0;
			break;
		case rWait:
			outputRec = 0;
			if(PIND & 0x01){//while high
				state = rWait;
			}
			else {
				state = rBuf;
				input = 0;				
				rCount = 0;
			}
			break;
		case rBuf:
			state = receive;
			break;
		case receive:
			if(rCount > 4){
				state = rWait;
				//input = input >> 4;	
				//rCount -= 4;
				outputRec = 1;
			}
			else {
				state = receive;
			}
			break;
	}
	switch (state){//state actions
		case rInit:
			break;
		case rWait:
			break;
		case receive:
			rCount++;
			input = input << 1;
			input += (PIND & 0x01);
			break;
	}

	return state;
}

int combineTick(int state){
	switch(state){
		case cInit:
			state = combine;
			break;
		case combine:
			state = combine;
			break;
		}
	switch (state){
		case cInit: break;
		case combine:
			if(outputRec){
				outputRec = 0;
				PORTC = input;
				nokia_lcd_clear();
				if(rCount != 5){
					nokia_lcd_write_string("no match", 1);
				}
				else {
					if(input == 0){
						nokia_lcd_write_char('5', 1);
					}
					else if(input == 1){
						nokia_lcd_write_char('4', 1);
					}
					else if(input == 3){
						nokia_lcd_write_char('3', 1);
					}
					else if(input == 7){
						nokia_lcd_write_char('2', 1);
					}
					else if(input == 15){
						nokia_lcd_write_char('1', 1);
					}
					else if(input == 16){
						nokia_lcd_write_char('6', 1);
					}
					else if(input == 24){
						nokia_lcd_write_char('7', 1);
					}
					else if(input == 28){
						nokia_lcd_write_char('8', 1);
					}
					else if(input == 0x1E){
						nokia_lcd_write_char('9', 1);
					}
					else if(input == 0x1F){
						nokia_lcd_write_char('0', 1);
					}
					else {
						nokia_lcd_write_string("no match", 1);
					}
			}
		}
	nokia_lcd_render();
	}
	return state;
}
void initArr(){
	int i = 0;
    tasks[i].state = rInit;
    tasks[i].period = 100;
    tasks[i].elapsedTime = 100;
  	tasks[i].TickFct = &rTick;

  	i++;
  	tasks[i].state = cInit;
    tasks[i].period = 100;
    tasks[i].elapsedTime = 100;
  	tasks[i].TickFct = &combineTick;

 }

