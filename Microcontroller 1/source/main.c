#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#endif
enum srStates{srInit, srWait, srPress, srRelease} srState;
enum combineStates{cInit, combine}cState;
enum timeStates{tInit, tReg, tShift}tState;
enum UARTState{uInit, uWait, uSend, uFinish, uBuf}uState;


typedef struct task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;
unsigned char finish[4] = {1,0,1,0};
task tasks[4];
unsigned char tasksNum = 4;

int srTick(int);	unsigned char srOut = 0; unsigned char currIn = 0;
int srTime(int);	unsigned char enable = 0; unsigned char send = 0;
int uTick(int);		unsigned char uOut = 0xFF, uIn = 0, uCounter = 5; 
int combineTick(int);

void initArr();

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRB = 0xFF; PORTB = 0xFF; 
    /* Insert your solution below */
	initArr();
  	
  	TimerOn();
  	TimerSet(100);
  	unsigned char tasksPeriodGCD = 100;
  	
  	PORTC = 0x04;
 
    while (1) {
		for (int i=0; i < tasksNum; i++) {
			PINA = PINA & 0x0f;
    	    if (tasks[i].elapsedTime >= tasks[i].period){
		    tasks[i].state = tasks[i].TickFct(tasks[i].state); 
		    tasks[i].elapsedTime = 0; // Reset the elapsed time
	    }
	    tasks[i].elapsedTime += tasksPeriodGCD;
	   
		}
			
		while(!TimerFlag);
		TimerFlag = 0;	
    }
    return 1;
}

int srTick(int state){
	switch(state){
		case srInit:
			srOut = 0x05;
			state = srWait;
			break;
		case srWait:
			if(PINA) state = srPress;
			else state = srWait;
			break;
		case srPress:
			state = srRelease;
			break;
		case srRelease:
			enable = 0;
			if((PINA & 0x0F) == 0) {state = srWait; srOut = 0x04;}
			else state = srRelease;
			break;
	}
	switch (state){
		case srInit: break;
		case srWait: break;
		case srRelease: 
			//currIn = 0;
			break;
		case srPress:
			enable = 2;
			if(PINA & 0x01){
				uCounter++;
				srOut = srOut| 0x00;//dot
				currIn = currIn << 1;
			}
			else if(PINA & 0x02){
				uCounter++;
				srOut = srOut|0x01;//dash
				currIn = currIn << 1;
				currIn++;
			}
			if(PINA & 0x04){//send and clear;
				srOut = 0x02;
				send = 1;
				if(currIn == 0){
					uCounter = 1;
				}
				currIn = 0;
			}
			break;
	}
	
	return state;
}

int srTime(int state){
	switch (state){
		case tInit:
			state = tShift;
			break;
		case tShift:
			if(enable){
				enable--;
				state = tReg;
				srOut = srOut & 0x07;
				srOut = srOut | 0x02;
			}
			else{state = tShift;}
			break;
		case tReg:
			if(enable){
				enable--;
				state = tShift;
				srOut = srOut & 0x0D;
				srOut = srOut| 0x08;
			}
			else state = tReg;
			break;
		}
	switch (state){
		
	}
	return state;
}

int uTick(int state){
	switch (state){
		case uInit:
			state = uWait;
			break;
		case uWait:
			if(!(send)){
				state = uWait;	
			}
			else{
				send = 0;
				uOut = 0x00;
				state = uBuf;
			}
			break;
		case uSend:
			if(uCounter > 0){
				state = uSend;
			}
			else{
				state = uWait;
				uCounter = 0;
			}
			break;
		case uBuf:
			state = uSend;
			break;
		/*
		case uFinish:
			if(uCounter < 1){ state = uFinish;}
			else {
				state = uWait;
				uOut = 0xFF;
				uCounter = 0;
			}
			break;
		*/
	}
	switch(state){
		case uInit: break;
		//case uBuf: uOut = 0x00; break;
		case uWait:
			if(!(PINA&0x04)){
				uIn = currIn;	
			}
			uOut = 0xFF;
			break;
		case uSend:
			uCounter--;
			uOut = (uIn >> (uCounter)) & 0x01;
			break;
		/*case uFinish:
			//uOut = finish[uCounter];
			uCounter++;
			*/
	}
	return state;
			
}

int combineTick(int state){
	switch (state){
		case cInit:
			state = combine;
			break;
		case combine:
			state = combine;
			break;
		}
	switch (state){
		case combine:
			PORTC = srOut;
			PORTB = uOut;
			break;
	}
	return state;
}


void initArr(){
	int i = 0;
    tasks[i].state = srInit;
    tasks[i].period = 100;
    tasks[i].elapsedTime = 100;
  	tasks[i].TickFct = &srTick;

  	i++;
  	tasks[i].state = uInit;
    tasks[i].period = 100;
    tasks[i].elapsedTime = 100;
  	tasks[i].TickFct = &uTick;

  	i++;
  	tasks[i].state = cInit;
    tasks[i].period = 100;
    tasks[i].elapsedTime = 100;
  	tasks[i].TickFct = &combineTick;

  	i++;
  	tasks[i].state = tInit;
    tasks[i].period = 100;
    tasks[i].elapsedTime = 100;
  	tasks[i].TickFct = &srTime;
  	
 }

	
