#include "msp.h"
#include "delay.h"

void set_DCO(int freq) {
   /* Sets the DCO to the desired frequency. Expects one of the #defined values */
   /* This function currently does not return Vcore or Flash controller back
      to the default state if transitioning from 48Mhz to another frequency */
   
   CS->KEY = CS_KEY_VAL;	/* unlock CS reg */
   CS->CTL0 = 0;		/* clear reg */

   switch(freq) {
      case FREQ_1_5MHz:
	 CS->CTL0 = CS_CTL0_DCORSEL_0;
	 break;
      case FREQ_3MHz:
	 CS->CTL0 = CS_CTL0_DCORSEL_1; 
	 break;
      case FREQ_6MHz:
	 CS->CTL0 = CS_CTL0_DCORSEL_2;
	 break;
      case FREQ_12MHz:
	 CS->CTL0 = CS_CTL0_DCORSEL_3;
	 break;
      case FREQ_24MHz:
	 CS->CTL0 = CS_CTL0_DCORSEL_4;
	 break;

      case FREQ_48MHz:
	 prepareFor48Mhz();
	 CS->CTL0 = CS_CTL0_DCORSEL_5;
	 break;
   }

   CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3;
   CS->KEY = 0;
}


void prepareFor48Mhz(){
   /* This function does the necessary adjustments to allow 
      the clock to be set to 48 Mhz */
   
   /* Transition to VCORE Level 1: AM0_LDO --> AM1_LDO */
   while ((PCM->CTL1 & PCM_CTL1_PMR_BUSY));
   PCM->CTL0 = PCM_CTL0_KEY_VAL | PCM_CTL0_AMR_1;
   while ((PCM->CTL1 & PCM_CTL1_PMR_BUSY));

   /* Configure Flash wait-state to 1 for both banks 0 & 1 */
   FLCTL->BANK0_RDCTL = (FLCTL->BANK0_RDCTL &
			 ~(FLCTL_BANK0_RDCTL_WAIT_MASK)) | FLCTL_BANK0_RDCTL_WAIT_1;
   FLCTL->BANK1_RDCTL = (FLCTL->BANK0_RDCTL &
			 ~(FLCTL_BANK1_RDCTL_WAIT_MASK)) | FLCTL_BANK1_RDCTL_WAIT_1;


   /* Configure DCO to 48MHz, ensure MCLK uses DCO as source*/
   CS->KEY = CS_KEY_VAL ;    //Unlock CS module for register access
   CS->CTL0 = 0;             //Reset tuning parameters
   CS->CTL0 = CS_CTL0_DCORSEL_5; //Set DCO to 48MHz

	 
 
}


void delay_ms(int time, int freq) {
   /* delays requested time in milliseconds using a busy loop */
   int i;
   /* for(i= time*(freq/10000); i > 0; i--) */
   for(i= time*(freq/9900); i > 0; i--)
      ;
}

void delay_us(int time, int freq) {
   /* delays requested time in microseconds using a busy loop */
   int i;
   for(i= time*(freq/11000)/1000; i > 0; i--)
      ;
}

