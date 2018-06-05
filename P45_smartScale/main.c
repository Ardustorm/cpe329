#include "msp.h"
#include "myLibs/uart.h"
#include "myLibs/delay.h"
#include <strings.h>
/**
 * main.c
 */


/* 
264000 2381 g
20500  184

zero ~ -50 -- 100

2490 22
23730  212

 */

int32_t offset=0;

void printNum(float num) {
   char buf[32];
   char *c;

   sprintf(buf, "%f\r\n", num);
   
   c= buf;
   while( *c != '\0') {
      EUSCI_A0->TXBUF = *c;

      uint32_t i;
      for(i=0; i<10000; i++);

      c++;
   }
}

void scaleInit() {
   /* P6->DIR |=  BIT0;		/\* Clk *\/ */
   /* P6->DIR &= ~BIT1;		/\* Data IN *\/ */
   /* P6->REN |=  BIT1;		/\* Data IN *\/ */

   P10->DIR |=  BIT0;		/* Clk */
   P10->DIR &= ~BIT2;		/* Data IN */
   P10->REN |=  BIT2;		/* Data IN */

   P10->OUT &=  ~BIT0;

   /* vcc */
   P7->DIR |= BIT6;
   P7->OUT |= BIT6;
   /* gnd */
   P10->DIR |=  BIT4;
   P10->OUT &= ~BIT4;
      
   int i;
   for(i=0; i<100000; i++);

   offset=getVals(10);

   
}


int getVal() {
   uint32_t Count;
   uint8_t  i;
   /* ADDO=1; */

   P10->OUT &=  ~BIT0;		/*    ADSK=0; */

   Count=0;
   
   /* wait for DOUT to be low */
   while( P10->IN & BIT2);	/*    while(ADDO); */

   for (i=0;i<24;i++){
      P10->OUT |=  BIT0;		/*    ADSK=1; */
      Count=Count<<1;
      P10->OUT &=  ~BIT0;		/* ADSK=0; */
      if( P10->IN & BIT2) Count++;      	/* if(ADDO) Count++; */
   }
   P10->OUT |=  BIT0;		/*    ADSK=1; */

   Count=Count^0x800000;
   P10->OUT &=  ~BIT0;		/* ADSK=0; */
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

      printNum( 0.00902 * getSmoothVal());
      
      uint32_t i;
      
      for(i=0; i<100000; i++);
   }	    
}
