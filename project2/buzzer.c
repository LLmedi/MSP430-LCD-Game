#include <msp430.h>
#include "libTimer.h"
#include "buzzer.h"

//Setting P2SEL2.6, P2SEL2.7, P2SEL.7 as 0
//and P2SEL.6 as 1
void buzzer_init(){
  timerAUpmode();            //used to drive speaker
  P2SEL2 &= ~(BIT6 | BIT7);
  P2SEL &= ~BIT7;
  P2SEL |= BIT6;
  P2DIR = BIT6;              //enable output to speaker

}

void buzzer_set_period(short cycles){
  CCR0 = cycles;
  CCR1 = cycles >> 1;
}

void play_note(int note){
  buzzer_set_period(note);
}

