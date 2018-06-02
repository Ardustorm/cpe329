#include "msp.h"
#include "myLibs/delay.h"
#include "myLibs/uart.h"	/* TODO: please don't do this */
#include "adcSample.h"

void printFloat(float val) {
   /* currently, this function must take a float less than 10
      and will only print out 2 decimal places*/
   int i;
   for(i=4; i>0; i-- ) {
      while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));
      EUSCI_A0->TXBUF = '0' + (int)val;
      val -= (int)val;
      val *= 10;

      if(i==4) {		/* print decimal */
	 while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));
	 EUSCI_A0->TXBUF = '.';
      }
   }
}

int main(void) {
   volatile unsigned int i;

   set_DCO(FREQ_12MHz);
   uartInit();

   WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;  // Stop WDT

   initAdc();

   // Enable global interrupt
   __enable_irq();

   SCB->SCR &= ~SCB_SCR_SLEEPONEXIT_Msk;   // Wake up on exit from ISR
   startAdcSample();
   while (1)
      {
	 if( newAdcSample() ) {
	    startAdcSample();
	    while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));
	    EUSCI_A0->TXBUF = '>';
	    printFloat( currentAdcSample());
	    EUSCI_A0->TXBUF = '\r';
	    while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));
	    EUSCI_A0->TXBUF = '\n';
	 }
	 
      }
}


