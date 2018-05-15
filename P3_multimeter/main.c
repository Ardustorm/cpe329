#include "msp.h"
#include "myLibs/delay.h"
#include "myLibs/uart.h"
/**
 * main.c
 */

volatile uint32_t period = 0;
void main(void)
{
   WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
   set_DCO(FREQ_48MHz);
   uartInit();
   int i;
   for(i = 0; i < 30000; i++);

   volatile char * buf = malloc(500);
   strcpy(buf,
      LOC(5,22) "DC"      LOC(5,42) "RMS"        LOC(5,62)"PP" /* ends at 31 */
      LOC(6,20) "0.000 V" LOC(6,40) "0.000 Vrms" LOC(6,60)"0.000 Vpp"

	  );
   setOutput(buf);
   while(1) {
      int i;
      for(i = 0; i < 90000; i++);
      buf[35]++;


   }
}

/******************** BELOW NOT USED!!!  ********************/
void edgeTriggerInit() {
   P2->SEL0 |= BIT5;                       // TA0.CCI2A input capture pin, second function
   P2->DIR &= ~BIT5;

   // Timer0_A3 Setup
   TIMER_A0->CCTL[2] = TIMER_A_CCTLN_CM_1 | // Capture rising edge,
      TIMER_A_CCTLN_CCIS_0 |          // Use CCI2A=ACLK,
      TIMER_A_CCTLN_CCIE |            // Enable capture interrupt
      TIMER_A_CCTLN_CAP |             // Enable capture mode,
      TIMER_A_CCTLN_SCS;              // Synchronous capture

   TIMER_A0->CTL |= TIMER_A_CTL_TASSEL_1 | // Use ACLK as clock source,
      TIMER_A_CTL_MC_2 |              // Start timer in continuous mode
      TIMER_A_CTL_CLR;                // clear TA0R
}

void TA0_N_IRQHandler(void) {
   /* TODO: improve response time */
   /* perform exponential averaging */
   period += TIMER_A0->CCR[2];
   period /= 2;
   // Clear the interrupt flag
   TIMER_A0->CCTL[2] &= ~(TIMER_A_CCTLN_CCIFG);
}
