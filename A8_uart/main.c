#include "msp.h"
#include "myLibs/delay.h"
#include "myLibs/dac.h"
#include "myLibs/uart.h"

int main(void) {
   int value;
   WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;   // Stop watchdog timer

   set_DCO(FREQ_12MHz);
   setupDAC();
   uartInit();

   // Enable global interrupt
   __enable_irq();

   while(1) {
      if( (value = getUartNum()) >= 0 && (value < 4096) )
	 setDAC(value);
   }
}

