#include "msp.h"
#include "myLibs/uart.h"
#include "myLibs/delay.h"
#include <strings.h>
/**
 * main.c
 */

int32_t offset=0;

void printNum(int num) {
   char buf[255];
   char *c;

   sprintf(buf, "%i\r\n", num);
   
   c= buf;
   while( *c != '\0') {
      EUSCI_A0->TXBUF = *c;

      uint32_t i;
      for(i=0; i<10000; i++);

      c++;
   }
}

void scaleInit() {
   P6->DIR |=  BIT0;		/* Clk */
   P6->DIR &= ~BIT1;		/* Data IN */
   P6->REN |=  BIT1;		/* Data IN */

   P6->OUT &=  ~BIT0;

   offset=getVals(10);

   
}


int getVal() {
   uint32_t Count;
   uint8_t  i;
   /* ADDO=1; */

   P6->OUT &=  ~BIT0;		/*    ADSK=0; */

   Count=0;
   
   /* wait for DOUT to be low */
   while( P6->IN & BIT1);	/*    while(ADDO); */

   for (i=0;i<24;i++){
      P6->OUT |=  BIT0;		/*    ADSK=1; */
      Count=Count<<1;
      P6->OUT &=  ~BIT0;		/* ADSK=0; */
      if( P6->IN & BIT1) Count++;      	/* if(ADDO) Count++; */
   }
   P6->OUT |=  BIT0;		/*    ADSK=1; */

   Count=Count^0x800000;
   P6->OUT &=  ~BIT0;		/* ADSK=0; */
   return(Count - offset);
   
}

int getVals(int num) {
   uint32_t total=0;
   int i;
   for(i=0; i<num; i++) {
      total += getVal();
   }
   return total/num;
}

int getSmoothVal() {
   static int32_t expAvg=0;
   expAvg += getVal();
   expAvg /= 2;
   return expAvg;
}

void main(void) {
   WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
   set_DCO(FREQ_48MHz);
   uartInit();
   scaleInit();
   int index = 1;
   while(1) {
      index += 1234;
      /* EUSCI_A0->TXBUF = '.'; */

      printNum(getSmoothVal());
      
      uint32_t i;
      
      for(i=0; i<100000; i++);
   }	    
}
