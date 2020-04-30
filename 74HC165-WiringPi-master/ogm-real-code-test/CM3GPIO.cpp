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
    displayPinValues();

}




void CM3GPIO::poll(){

    // read keys (updates pinValues)
    shiftRegRead();

    // get key values if a key pin changed
    if (pinValues  != pinValuesLast ) {
         displayPinValues();
    }
   
    pinValuesLast = pinValues;

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


