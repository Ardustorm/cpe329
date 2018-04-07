#include "msp.h"

#include "delay.h"
/**
 * main.c
 */
#define FREQ_1_5MHz 1500000
#define FREQ_3MHz 3000000
#define FREQ_6MHz 6000000
#define FREQ_12MHz 12000000
#define FREQ_24MHz 24000000
#define FREQ_48MHz 48000000

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


void delay_accuracy(int freq) {
   /* Test function for measuring delay accuracy */
   set_DCO(freq);
   while(1) {
      P1->OUT ^= BIT0;

      delay_ms(1000, freq);
   }

}

void short_pulses(int freq) {
   /* test function consisting of 2 pulses */
   set_DCO(freq);

   P1->OUT &= ~BIT0;
   delay_us(100, freq);

   P1->OUT |= BIT0;
   delay_us(100, freq);

   P1->OUT &= ~BIT0;
   delay_us(100, freq);

   P1->OUT |= BIT0;
   delay_us(100, freq);

   P1->OUT &= ~BIT0;

}

void shortest_pulse(int freq) {
   /* Test function for seeing shortest pulse */
   set_DCO(freq);

   P1->OUT &= ~BIT0;
   delay_us(20, freq);		/* 20us == about 5% error */

   P1->OUT |= BIT0;
   delay_us(20, freq);
   P1->OUT &= ~BIT0;
   
}

void main(void)
{
   WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

   
   P1->SEL0 &= ~BIT0; //set gpio to general io
   P1->SEL1 &= ~BIT0;
   P1->DIR |= BIT0;

   /******************** TESTS ********************/
   /* set_DCO(FREQ_48MHz); */
   delay_accuracy(FREQ_24MHz);
   /* short_pulses(FREQ_48MHz); */
   /* shortest_pulse(FREQ_48MHz); */
}
