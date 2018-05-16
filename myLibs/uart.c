/* Code based of of TI uart example */

#include "uart.h"

static int uartFlag = 0;
static int num = 0;


static volatile int arrayStart=0;
static volatile char * outArray = CLR_SCREEN /* INVIS_CURSOR */;
/* LOC(5,20) "DC"     LOC(5,40) "RMS"    LOC(5,60)"PP"; */



/* void sendChar(char c) { */
/*    outArray[arrayEnd] = c; */
/*    arrayEnd = (arrayEnd +1) % ARRAY_SIZE; */
/* } */


/* returns the number if the uart flag is set, otherwise -1.
   The function clears the flag and the number after each call.
*/
int getUartNum() {
   if(uartFlag != 0) {
      int numTmp = num;
      num=0;
      uartFlag = 0;
      return numTmp;
   } else {
      return -1;
   }
}

void setOutput(char * out) {
   outArray=out;
   arrayStart=0;
}




void uartInit() {
   // Configure UART pins
   P1->SEL0 |= BIT2 | BIT3;                // set 2-UART pin as secondary function

   // Configure UART
   EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST; // Put eUSCI in reset
   EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SWRST | // Remain eUSCI in reset
      EUSCI_B_CTLW0_SSEL__SMCLK|      // Configure eUSCI clock source for SMCLK
      EUSCI_A_IE_TXIE;		      /* enable tx interrupt */

   // Baud Rate calculation
   EUSCI_A0->BRW = 78;
   EUSCI_A0->MCTLW = ((2 << EUSCI_A_MCTLW_BRF_OFS) |
		      EUSCI_A_MCTLW_OS16 |
		      (0x00 << EUSCI_A_MCTLW_BRS_OFS));

   EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST; // Initialize eUSCI
   EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;    // Clear eUSCI RX interrupt flag
   EUSCI_A0->IE |= EUSCI_A_IE_RXIE|        // Enable USCI_A0 RX interrupt
      EUSCI_A_IE_TXIE;			   /* enable TX interrupt */

   // Enable eUSCIA0 interrupt in NVIC module
   NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);


}


// UART interrupt service routine
void EUSCIA0_IRQHandler(void) {
   if (EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG) {
      EUSCI_A0->IFG &=~ EUSCI_A_IFG_RXIFG;
	    
      // Check if the TX buffer is empty first
      while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));

      /* check if number */
      if( '0' <= EUSCI_A0->RXBUF && EUSCI_A0->RXBUF <= '9') {
   	 // Echo the received character back
   	 EUSCI_A0->TXBUF = EUSCI_A0->RXBUF;
   	 num = 10*num + (EUSCI_A0->RXBUF - '0');
	 
      } else if(EUSCI_A0->RXBUF == '\r') {
   	 EUSCI_A0->TXBUF = '\n';
   	 EUSCI_A0->TXBUF = '\r';
   	 uartFlag = 1;
      }
   } else if (EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG) {
      /* finished sending */
      EUSCI_A0->TXBUF = outArray[arrayStart++];
      if(outArray[arrayStart] == '\0') {
      	 arrayStart = 0;
      }
      /* arrayStart = (arrayStart+1) % ARRAY_SIZE; */
   }
   
}
