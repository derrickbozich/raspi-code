//g++ -Wall -l wiringPi -o "CM3GPIO" "CM3GPIO.cpp" 
#include "CM3GPIO.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <sys/stat.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>


// GPIO pin defs
#define SR_DATA_WIDTH 32        // number of bits to shift in on the 74HC165s
#define SR_PLOAD 27             // parallel load pin 
#define SR_CLOCK_ENABLE 25      // CE pin 
#define SR_DATA 26              // Q7 pin 
#define SR_CLOCK 18             // CLK pin 



CM3GPIO::CM3GPIO() {
}

void CM3GPIO::init(){
    // setup GPIO, this uses actual BCM pin numbers 
    wiringPiSetupGpio();

    // GPIO for shift registers
    pinMode(SR_PLOAD, OUTPUT);
    pinMode(SR_CLOCK_ENABLE, OUTPUT);
    pinMode(SR_CLOCK, OUTPUT);
    pinMode(SR_DATA, INPUT);
    digitalWrite(SR_CLOCK, LOW);
    digitalWrite(SR_PLOAD, HIGH);
    digitalWrite(SR_CLOCK_ENABLE, LOW);


    // keys
    keyStatesLast = 0;
  

    // get initial pin states
    shiftRegRead();
    pinValuesLast = pinValues;

}




void CM3GPIO::poll(){

    // read keys (updates pinValues)
    shiftRegRead();

    // get key values if a key pin changed
    if (pinValues  != pinValuesLast ) {
         displayPinValues();
         getKeys();
       
    }
   
    
    pinValuesLast = pinValues;

   
    
    // check encoder, gotta check every time for debounce purposes
    getEncoder();
    
}





uint32_t CM3GPIO::shiftRegRead(void)
{
    uint32_t bitVal;
    uint32_t bytesVal = 0;

    // so far best way to do the bit banging reliably is to
    // repeat the calls to reduce output clock frequency, like ad hoc 'nop' instructions
    // delay functions no good for such small times

    // load
    digitalWrite(SR_PLOAD, LOW);
    digitalWrite(SR_PLOAD, LOW);
    digitalWrite(SR_PLOAD, LOW);
    digitalWrite(SR_PLOAD, LOW);
    digitalWrite(SR_PLOAD, LOW);
    digitalWrite(SR_PLOAD, LOW);
    digitalWrite(SR_PLOAD, LOW);
    digitalWrite(SR_PLOAD, LOW);
    
    digitalWrite(SR_PLOAD, HIGH);
    digitalWrite(SR_PLOAD, HIGH);
    digitalWrite(SR_PLOAD, HIGH);
    digitalWrite(SR_PLOAD, HIGH);
    digitalWrite(SR_PLOAD, HIGH);
    digitalWrite(SR_PLOAD, HIGH);
    digitalWrite(SR_PLOAD, HIGH);
    digitalWrite(SR_PLOAD, HIGH);
    
    // shiftin
   for(int i = 0; i < SR_DATA_WIDTH; i++)
    {
        bitVal = digitalRead(SR_DATA);

        bytesVal |= (bitVal << ((SR_DATA_WIDTH-1) - i));

        digitalWrite(SR_CLOCK, HIGH);
        digitalWrite(SR_CLOCK, HIGH);
        digitalWrite(SR_CLOCK, HIGH);
        digitalWrite(SR_CLOCK, HIGH);
        digitalWrite(SR_CLOCK, HIGH);
        digitalWrite(SR_CLOCK, HIGH);
        digitalWrite(SR_CLOCK, HIGH);
        digitalWrite(SR_CLOCK, HIGH);
        
        digitalWrite(SR_CLOCK, LOW);
        digitalWrite(SR_CLOCK, LOW);
        digitalWrite(SR_CLOCK, LOW);
        digitalWrite(SR_CLOCK, LOW);
        digitalWrite(SR_CLOCK, LOW);
        digitalWrite(SR_CLOCK, LOW);
        digitalWrite(SR_CLOCK, LOW);
        digitalWrite(SR_CLOCK, LOW);
    }
    
    pinValues = bytesVal;
    return(bytesVal);
}

void CM3GPIO::getKeys(void){
    keyStates = 0;
    
    keyStates |= (pinValues >> (0 + 7) & 1) << 24;
    keyStates |= (pinValues >> (1 + 7) & 1) << 16;
    keyStates |= (pinValues >> (2 + 7) & 1) << 17;
    keyStates |= (pinValues >> (3 + 7) & 1) << 18;
    keyStates |= (pinValues >> (4 + 7) & 1) << 19;
    keyStates |= (pinValues >> (5 + 7) & 1) << 20;
    keyStates |= (pinValues >> (6 + 7) & 1) << 21;
    keyStates |= (pinValues >> (7 + 7) & 1) << 22;
   
    keyStates |= (pinValues >> (8 + 7) & 1) << 23;
    keyStates |= (pinValues >> (9 + 7) & 1) << 8;
    keyStates |= (pinValues >> (10 + 7) & 1) << 9;
    keyStates |= (pinValues >> (11 + 7) & 1) << 10;
    keyStates |= (pinValues >> (12 + 7) & 1) << 11;
    keyStates |= (pinValues >> (13 + 7) & 1) << 12;
    keyStates |= (pinValues >> (14 + 7) & 1) << 13;
    keyStates |= (pinValues >> (15 + 7) & 1) << 14;
    
    keyStates |= (pinValues >> (16 + 7) & 1) << 15;
    keyStates |= (pinValues >> (17 + 7) & 1) << 0;
    keyStates |= (pinValues >> (18 + 7) & 1) << 1;
    keyStates |= (pinValues >> (19 + 7) & 1) << 2;
    keyStates |= (pinValues >> (20 + 7) & 1) << 3;
    keyStates |= (pinValues >> (21 + 7) & 1) << 4;
    keyStates |= (pinValues >> (22 + 7) & 1) << 5;
    keyStates |= (pinValues >> (23 + 7) & 1) << 6;
    
    keyStates |= (pinValues >> (24 + 7) & 1) << 7;
    
    keyStates |= (0xFE000000);  // zero out the bits not key bits
    keyStates = ~keyStates;
}

void CM3GPIO::getEncoder(void){

	static uint8_t encoder_last = 0;
	uint8_t encoder = 0;

	#define PRESS 0
	#define RELEASE 1
	uint8_t button;
	static uint8_t button_last = RELEASE;
	static uint8_t press_count = 0;
	static uint8_t release_count = 0;

	button = (pinValues >> 4) & 0x1;
	if (button == PRESS) {
		press_count++;
		release_count = 0;
	}
	if ((press_count > 10) && (button_last == RELEASE)){	// press
			button_last = PRESS;
			release_count = 0;
            encBut = 1;
            encButFlag = 1;
	}

	if (button == RELEASE) {
		release_count++;
		press_count = 0;
	}
	if ((release_count > 10) && (button_last == PRESS)){	// release
			button_last = RELEASE;
			press_count = 0;
            encBut = 0;
            encButFlag = 1;
	}

	// turning
	encoder = (pinValues >> 5) & 0x3;
	
    if (encoder != encoder_last) {
        if (encoder_last == 0) {
	    if (encoder == 2){
	        encTurn = 1;
                encTurnFlag = 1;
            }
            if (encoder == 1){
                encTurn = 0;
                encTurnFlag = 1; 
	    }
        }
        if (encoder_last == 3) {
	    if (encoder == 1){
                encTurn = 1;
                encTurnFlag = 1;
	    }
            if (encoder == 2){
                encTurn = 0;
                encTurnFlag = 1;
            }
        }
        encoder_last = encoder;
	}
}



void CM3GPIO::displayPinValues(void)
{
    for(int i = 0; i < SR_DATA_WIDTH; i++)
    {
        printf(" ");

        if((pinValues >> ((SR_DATA_WIDTH-1)-i)) & 1)
            printf("1");
        else
            printf("0");

    }
    printf("\n");
}

CM3GPIO controls;


int main() {
 
    controls.init();

    for (;;) {
      

        // check for events from hardware controls
        controls.poll();

        // main polling loop delay
        // slow it down for cm3 cause all the bit banging starts to eat CPU

        usleep(2000);

    } // for;;
}


