#include "msp.h"
#include "myLibs/delay.h"
/**
 * main.c
 * P1.5  --  Sclk
 * P1.6  --  MOSI
 * P1.7  --  MISO
 * P4.1  --  CS for DAC

 */


#define CUR_CLOCK_FREQ FREQ_3MHz
#define CUR_CLOCK_DIV 1

#define MAX_DUTY_CYCLE 1000
#define VDD 3300              //mV
#define ADC_RES 4096
#define TRI_STEPS 1000

#define SIN_SIZE 4096

uint32_t SIN_DELAYS[] = {0};//0,4095];
uint16_t SIN_OUTPUT[] = {0};//GIANT ARRAY];


uint32_t nsToClocks(uint32_t nsec){
   return nsec/10*CUR_CLOCK_FREQ/CUR_CLOCK_DIV;
}

// sizeof 2, sizeof 2,mV,mV, HZ, 0-1000
void gen_square(volatile uint32_t *t, volatile uint32_t *v, uint16_t Vpp, uint16_t Vdc, uint32_t freq, uint16_t duty) {
   t[0]=nsToClocks( (MAX_DUTY_CYCLE-duty) / freq / MAX_DUTY_CYCLE );
   t[1]=nsToClocks((duty)/freq/MAX_DUTY_CYCLE);
   v[0]=0x0823;//Vdc*ADC_RES/VDD-Vpp*ADC_RES/VDD/2;
   v[1]=0x0fed;//Vdc*ADC_RES/VDD+Vpp*ADC_RES/VDD/2;
}
void gen_sin(volatile uint32_t *t, volatile uint32_t *v, uint16_t Vpp, uint16_t Vdc, uint32_t freq) {
   uint16_t i;
   for(i=0; i<SIN_SIZE; i++) {
      t[i]=nsToClocks(SIN_DELAYS[i]/freq);
      v[i]=SIN_OUTPUT[i]*Vpp/VDD+(Vdc-VDD/2);  //scale and add offset
   }
}

void gen_tri(volatile uint32_t *t, volatile uint32_t *v, uint16_t Vpp, uint16_t Vdc, uint32_t freq, uint16_t duty) {
   uint32_t peakI;
   uint32_t peakV;
   uint16_t i;
   for(i=0;i<TRI_STEPS;i++){
      t[i]=nsToClocks(1/TRI_STEPS/freq);
   }
   peakI=TRI_STEPS*duty/MAX_DUTY_CYCLE;
   peakV=Vdc*ADC_RES/VDD+Vpp*ADC_RES/VDD/2;
   for(i=0;i<peakI;i++){
      v[i]=i*peakV/peakI;
   }
   for(i=peakI;i<TRI_STEPS;i++){
      v[i]=peakV*(1+(peakI-i)/peakI);
   }
}


























volatile uint16_t waveIndex=0;
volatile uint16_t voltages[2];
volatile uint32_t times[2];

void main(void)
{
   uint16_t data;
   uint32_t i;
   WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

   setupDAC();
   a6SquareWave();


   /*** TIMER INTERRUPTS ****/
   TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
   TIMER_A0->CCR[0] = 1240;
   TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK | // SMCLK, continuous mode
      TIMER_A_CTL_MC__CONTINUOUS;

   SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;    // Enable sleep on exit from ISR
   NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);

   
   __enable_irq();

   NVIC->ISER[0] = 1 << ( (EUSCIB0_IRQn) & 31);
   

   while(1) {
   }
}


void TA0_0_IRQHandler(void) {
   TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
   static int index =0;

   /* SQUARE WAVE */
   voltages[0]=0;
   voltages[1]=2482;
   times[0]=30000;
   times[1]=30000;

   setDAC( voltages[index]);
   TIMER_A0->CCR[0] += times[index];
   index = (index+1) % 2;


   
   /* /\* TRI WAVE *\/ */
   /* static int dir =1; */
   /* if(index >= 2150){ */
   /*    dir = -1; */
   /* } else if(index <= 0){ */
   /*    dir= 1; */
   /* } */
   
   /* setDAC( index );    */
   /* TIMER_A0->CCR[0] += 280; */
   /* index += dir*20; */


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
