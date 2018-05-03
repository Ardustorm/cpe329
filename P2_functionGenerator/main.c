#include "msp.h"
#include "myLibs/delay.h"
#include "waveforms.h"
/**
 * main.c
 * P1.5  --  Sclk
 * P1.6  --  MOSI
 * P1.7  --  MISO
 * P4.1  --  CS for DAC

 */

#define CUR_CLOCK_FREQ FREQ_12MHz

/* #define FREQ_CUR FREQ_12MHz */

volatile int waveFreq = 1;
volatile int waveType = 0;		/* Square:0,  Sawtooth:1, Sine:2, inverseSaw:3 */
volatile int waveDuty = 5;		/* in perdeci */

int key;
volatile void main(void)
{

   P1->DIR |= BIT0;
   P1->OUT &= ~BIT0;
   P1->OUT |= BIT0;
   
   WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

   /* set_DCO(CUR_CLOCK_FREQ); */
   setupDAC();
   keypadInit();
   LCD_Init();
   /*** TIMER INTERRUPTS ****/
   TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
   TIMER_A0->CCR[0] = 1240;
   TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK | // SMCLK, continuous mode
      TIMER_A_CTL_MC__CONTINUOUS;

   /* SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;    // Enable sleep on exit from ISR */
   NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);


   
   __enable_irq();
   
   Clear_LCD();   
   while(1) {

      Write_str_LCD(0,"Freq: ");
      Write_char_LCD(6,'0'+waveFreq);
      Write_str_LCD(7,"00 Hz");

      if(waveType == 0) {
	 Write_char_LCD(29, '0'+waveDuty);
	 Write_str_LCD(30, "0%");
      }
      key=getKey();

      
      if(key > 0)
      	 Write_char_LCD(0,key);
      switch(key) {
      	 case -1:
      	    break;
	 
      	    /* DUTY CYCLE */
      	 case '*':
      	    waveDuty = (waveDuty-1 >0) ? waveDuty-1 : 1;
	    while(scanKeypad() != -1);
      	    break;
      	 case '0':
      	    waveDuty=5;
      	    break;
      	 case '#':
      	    waveDuty = (waveDuty <9) ? waveDuty+1 : 9;
	    while(scanKeypad() != -1);
      	    break;



      	    /* Wave Types */
      	 case '6':
      	    waveType = 3;	/* Arb Wave */
	    Clear_LCD();
	    Write_str_LCD(16,"Type: Arb");
      	    break;
      	 case '7':
      	    waveDuty=5;
      	    waveType = 0;	/* Square */
	    Clear_LCD();
	    Write_str_LCD(16,"Type: Square");
	    
      	    break;
      	 case '8':
      	    waveType = 1;	/* Sawtooth */
	    Clear_LCD();
	    Write_str_LCD(16,"Type: Sawtooth");

      	    break;
      	 case '9':
      	    waveType = 2;	/* Sine */
	    Clear_LCD();
	    Write_str_LCD(16,"Type: Sine");
      	    break;

      	 default:
      	    waveFreq = key -'0';
      	    break;
      }

   }

	
}


volatile void TA0_0_IRQHandler(void) {
   TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
   static int index =0;

   
   if( waveType == 2) {
      setDAC(SIN_WAVE[index]);

      index = (index + waveFreq) % SIN_LEN;
      TIMER_A0->CCR[0] += CLK_STEP;
      return;
   } else if(waveType== 1) {
      
      setDAC(index*4096/SIN_LEN);
      index = (index + waveFreq) % SIN_LEN;
      TIMER_A0->CCR[0] += CLK_STEP;
      return;
   } else if(waveType== 3) {
      
      setDAC(index*4096/SIN_LEN);
      index = (index - waveFreq) % SIN_LEN;
      TIMER_A0->CCR[0] += CLK_STEP;
      return;
   } else if(waveType== 0) {
      /* SQUARE WAVE */
      index = index ? 0:4095;
      setDAC( index);
      
      TIMER_A0->CCR[0] +=  (index? (60000 / waveFreq *waveDuty/5) :
			    (60000 / waveFreq *(10-waveDuty)/5));

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
