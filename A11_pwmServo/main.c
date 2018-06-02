#include "msp.h"
#include "myLibs/keypad.h"


volatile int positionInClocks = 4500;
volatile void updatePosition(int pos) {
   pos = (pos < 0)? 0: pos;
   pos = (pos > 180)? 180: pos;
   positionInClocks = (((180-pos) /180.0 * 1.8) +0.6) * 3000.0;
}

int main(void) {
   WDT_A->CTL = WDT_A_CTL_PW |             // Stop WDT
      WDT_A_CTL_HOLD;

   // Configure GPIO
   P4->DIR |= BIT0;
   P4->OUT |= BIT0;

   /* keypad Init */
   keypadInit();
    
   TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
   TIMER_A0->CCR[0] = 60000;
   TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK | // SMCLK, continuous mode
      TIMER_A_CTL_MC__CONTINUOUS;

   // Enable global interrupt
   __enable_irq();

   NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);

   char key='\0';
   uint8_t posTmp=0;		/* stores 1st digit when entering positon */
   int position = 90;
   uint8_t loc = 0;		/* which digit we're on */
   while (1)
      {
	 
	 P1->OUT |= BIT0;
	 key = getKey();
	 
	 if(key == '*') {
	    position -= 10;
	    loc = 0;
	 } else if(key == '#') {
	    position += 10;
	    loc = 0;
	 } else if(loc == 0) {
	    loc++;
	    posTmp = 100 * (key - '0');
	 } else {
	    loc = 0;
	    position = posTmp + 10*(key - '0');
	    posTmp=0;
	 }

	 updatePosition(position);

	 getKeyCombo();		/* wait till key released */
      }
}

// Timer A0 interrupt service routine

void TA0_0_IRQHandler(void) {

   
   TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
   if(P4->OUT & BIT0) {		  /* was high */
      TIMER_A0->CCR[0] += 60000 - positionInClocks;      // Add 20ms Offset to TACCR0
      P4->OUT &= ~BIT0;
   } else {
      TIMER_A0->CCR[0] += positionInClocks;
      P4->OUT |= BIT0;
   }
}


