/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2013, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 *                       MSP432 CODE EXAMPLE DISCLAIMER
 *
 * MSP432 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see http://www.ti.com/tool/mspdriverlib for an API functional
 * library & https://dev.ti.com/pinmux/ for a GUI approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//******************************************************************************
//  MSP432P401 Demo - Timer0_A3, Toggle P1.0, CCR0 Cont Mode ISR, DCO SMCLK
//
//  Description: Toggle P1.0 using software and TA_0 ISR. Timer0_A is
//  configured for continuous mode, thus the timer overflows when TAR counts
//  to CCR0. In this example, CCR0 is loaded with 50000.
//
//           MSP432P401x
//         ---------------
//     /|\|               |
//      | |               |
//      --|RST            |
//        |               |
//        |           P1.0|-->LED
//
//   William Goh
//   Texas Instruments Inc.
//   June 2016 (updated) | November 2013 (created)
//   Built with CCSv6.1, IAR, Keil, GCC
//******************************************************************************
#include "msp.h"
#include "myLibs/delay.h"

int state = 1;

int count = 0;

int main(void) {
   WDT_A->CTL = WDT_A_CTL_PW |             // Stop WDT
      WDT_A_CTL_HOLD;

   // Configure GPIO 25k clk
   P1->DIR |= BIT0;
   P1->OUT |= BIT0;

   /* config for 2bit clock */
   P2->DIR |= BIT0;
   P2->OUT |= BIT0;
   P2->DIR |= BIT1;
   P2->OUT |= BIT1;

   /* gpio timing of isr */
   P6->DIR |= BIT4;
   P6->OUT &= ~BIT4;

   /* mclk */
   P4->DIR |= BIT3;
   P4->SEL1&= ~BIT3;
   P4->SEL0|= BIT3;

   TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
   TIMER_A0->CCR[0] = 240;
   TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK | // SMCLK, continuous mode
      TIMER_A_CTL_MC__CONTINUOUS;

   SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;    // Enable sleep on exit from ISR



   /* STUFF for 20 sec */
   TIMER_A0->CTL |= BIT7 | BIT6; /* divide by 8 */
   TIMER_A0->EX0 |= 0x7;	 /* divide by 8 */

   /* CS->CTL1 |= CS_CTL1_DIVA__128; */

   /* set_DCO(FREQ_24MHz); */
     set_DCO(FREQ_1_5MHz);
    
   // Enable global interrupt
   __enable_irq();

   NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);

   while (1)
      {
	 __sleep();

	 __no_operation();                   // For debugger
      }
}



void TA0_0_IRQHandler_2bitcounter(void) {
/* void TA0_0_IRQHandler(void) { */
   TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;


   P2->OUT &= ~(BIT0 | BIT1);
   P2->OUT |= (0x3 & count);
   

   /* TIMER_A0->CCR[0] += 1831;              // Add Offset to TACCR0 */
   TIMER_A0->CCR[0] += 750;              // Add Offset to TACCR0
   count++;
}

/* void TA0_0_IRQHandler_20SEC(void) { */
void TA0_0_IRQHandler(void) {
   TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;


   if(count==8) {
      count=0;
      P1->OUT ^= BIT0;
   }
   /* TIMER_A0->CCR[0] += 1831;              // Add Offset to TACCR0 */
   TIMER_A0->CCR[0] += 29297;              // Add Offset to TACCR0
   count++;
}

// Timer A0 interrupt service routine
void TA0_0_IRQHandler_SHORTEST(void) {
/* void TA0_0_IRQHandler(void) { */
   TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;

   P1->OUT ^= BIT0;
   TIMER_A0->CCR[0] += 55;              // Add Offset to TACCR0
}


// Timer A0 interrupt service routine
void TA0_0_IRQHandler_50RATE(void) {
/* void TA0_0_IRQHandler(void) { */
   P6->OUT |= BIT4;
   TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;

   P1->OUT ^= BIT0;
   TIMER_A0->CCR[0] += 480;              // Add Offset to TACCR0

   P6->OUT &= ~BIT4;
}

void TA0_0_IRQHandler_25RATE(void) {
/* void TA0_0_IRQHandler(void) { */
   TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;

   if(state == 1) {		/* high */
      P1->OUT &= ~BIT0;
      TIMER_A0->CCR[0] += 240*3;              // Add Offset to TACCR0
      state=0;
   } else {
      P1->OUT |= BIT0;
      TIMER_A0->CCR[0] += 240*1;              // Add Offset to TACCR0
      state=1;
   }

}


