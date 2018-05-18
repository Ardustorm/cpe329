#include "dac.h"
void setupDAC(){
   P1->SEL0 |=  (BIT5 | BIT6 | BIT7); /* SPI def, pg 134 in datasheet */
   P4->DIR |= BIT1;		      /* Chip select */

   EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST; /* reset (probably pointless) */
   EUSCI_B0->CTLW0 = (EUSCI_B_CTLW0_SWRST    |
		      EUSCI_B_CTLW0_MST      |
		      EUSCI_B_CTLW0_SYNC     |
		      EUSCI_B_CTLW0_CKPL     |
		      EUSCI_B_CTLW0_UCSSEL_2 |
		      EUSCI_B_CTLW0_MSB      );
   EUSCI_B0->BRW = 0x0001;	/* set baudrate divisor to 1 */
   EUSCI_B0->CTLW0 &= ~EUSCI_B_CTLW0_SWRST; /* start state machine */

   EUSCI_B0->IE |= EUSCI_B_IE_RXIE;

}


void setDAC(uint16_t data) {
   P4->OUT &= ~(BIT1);
	 
   EUSCI_B0->TXBUF = 0x70 | ((0x0f) & (data>>8));
   EUSCI_B0->TXBUF = 0xff & data;

   P4->OUT |= (BIT1);
}
