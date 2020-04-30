#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <sys/stat.h>

#include "CM3GPIO.h"

// hardware interface controls
// default to organelle original

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

