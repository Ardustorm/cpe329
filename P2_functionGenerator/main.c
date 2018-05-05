#include "msp.h"
#include "myLibs/delay.h"
#include "waveforms.h"
#include "myLibs/lcd.h"
#include "myLibs/keypad.h"
#include <math.h>
#include <stdlib.h>
/**
 * main.c
 * P1.5  --  Sclk
 * P1.6  --  MOSI
 * P1.7  --  MISO
 * P4.1  --  CS for DAC

 */

#define CUR_CLOCK_FREQ FREQ_12MHz

#define FREQ_CUR FREQ_12MHz


#define SQUARE_WT 0
#define SAWTOOTH_WT 1
#define SINE_WT 2
#define ARB_WT 3
volatile int waveFreq = 1;
volatile int waveType = SQUARE_WT;		/* Square:0,  Sawtooth:1, Sine:2, inverseSaw:3 */
volatile int waveDuty = 5;		/* in perdeci */

int key;
volatile void main(void)
{

   P1->DIR |= BIT0;
   P1->OUT &= ~BIT0;
   P1->OUT |= BIT0;
   
   WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

   set_DCO(CUR_CLOCK_FREQ);
   setupDAC();
   keypadInit();
   LCD_Init();
   /*** TIMER INTERRUPTS ****/
   TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
   TIMER_A0->CCR[0] = 50240;
   TIMER_A0->CTL = (TIMER_A_CTL_SSEL__SMCLK | // SMCLK, continuous mode
		    TIMER_A_CTL_MC__CONTINUOUS);


   NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);

   generateArrays();
   
   __enable_irq();
   
   Clear_LCD();   
   while(1) {

      /* Write_str_LCD(0,"Freq: "); */
      /* Write_char_LCD(6,'0'+waveFreq); */
      /* Write_str_LCD(7,"00 Hz"); */

      /* if(waveType == SQUARE_WT) { /\* Display Duty cycle *\/ */
      /* 	 Write_char_LCD(29, '0'+waveDuty); */
      /* 	 Write_str_LCD(30, "0%"); */
      /* } */

      key=getKey();
      updateTypeAndFreq(key);
      

      /* wait till the key is released */
      while(scanKeypad() != -1);

   }

	
}


volatile void TA0_0_IRQHandler(void) {
   TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
   static int index =0;
   static int skip = 0;		/* used to skip every other sample for square wave */
   if(waveType != SQUARE_WT) {
      index = (index + waveFreq) % WAVE_LEN;
      setDAC(WAVES[waveType][index]);
      TIMER_A0->CCR[0] += CLK_STEP;
   } else if(waveType== SQUARE_WT) {
      /* SQUARE WAVE */
      if(skip == 0) {
      	 index = index ? 0:4095;
      	 setDAC( index);
      }
      skip ^= 0x1;
      
      TIMER_A0->CCR[0] +=  30000 / waveFreq *(index? ( waveDuty/5.0) :
					      ((10-waveDuty)/5.0));
   }
}



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



void updateTypeAndFreq(int key) {
   switch(key) {
      case -1:
	 break;
	 
	 /* DUTY CYCLE */
      case '*':
	 waveDuty = (waveDuty-1 >0) ? waveDuty-1 : 1;
	 break;
      case '0':
	 waveDuty=5;
	 break;
      case '#':
	 waveDuty = (waveDuty <9) ? waveDuty+1 : 9;
	 break;

	 /* Wave Types */
      case '6':
	 if(waveType >= 3) {
	    waveType = 4;
	 } else {
	    waveType = ARB_WT;	/* Arb Wave */
	 }
	 Clear_LCD();
	 Write_str_LCD(16,"Type: Arb");
	 break;
      case '7':
	 waveDuty=5;
	 waveType = SQUARE_WT;	/* Square */
	 Clear_LCD();
	 Write_str_LCD(16,"Type: Square");
	    
	 break;
      case '8':
	 waveType = SAWTOOTH_WT;	/* Sawtooth */
	 Clear_LCD();
	 Write_str_LCD(16,"Type: Sawtooth");

	 break;
      case '9':
	 waveType = SINE_WT;	/* Sine */
	 Clear_LCD();
	 Write_str_LCD(16,"Type: Sine");
	 break;

      default:
	 waveFreq = key -'0';
	 break;
   }
}


int abs(int a) {
   return( (a>0)? a : -a);
}

void generateArrays() {
   int peak=4095;
   int i;
   for( i=0; i<WAVE_LEN; i++) {
      SQUARE_WAVE[i] = (i<WAVE_LEN/2)? 0: peak;
      SIN_WAVE[i] = (int) (peak/2 + peak/2 * sin(i* 2* PI/WAVE_LEN));
      SAW_WAVE[i] = (int) (i*peak/WAVE_LEN);

      TRI_WAVE[i] = (int)(peak*(1-2.0/WAVE_LEN*abs(i-WAVE_LEN/2)));

      //(int) (i-2/WAVE_LEN* abs((int)(i-WAVE_LEN/2)));
      /* NOISE_WAVE[i] = random() * peak; */
   }

   WAVES[0] = SQUARE_WAVE;
   WAVES[1] = SAW_WAVE;
   WAVES[2] = SIN_WAVE;
   WAVES[3] = NOISE_WAVE;
   WAVES[4] = TRI_WAVE;
}
