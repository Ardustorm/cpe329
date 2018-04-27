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

#define MAX_DUTY_CYCLE = 1000
#define VDD 3300              //mV
#define ADC_RES 4096
#define TRI_STEPS 1000

#define SIN_SIZE = 4096;

uint32_t SIN_DELAYS[] = {0};//0,4095];
uint16_t SIN_OUTPUT[] = {0};//GIANT ARRAY];


uint32_t nsToClocks(uint32_t nsec){
   return nsec/1000*CUR_CLOCK_FREQ/CUR_CLOCK_DIV;
}

// sizeof 2, sizeof 2,mV,mV, HZ, 0-1000
void gen_square(uint32_t *t, uint32_t *v, uint16_t Vpp, uint16_t Vdc, uint32_t freq, uint16_t duty) {
   t[0]=nsToClocks( (MAX_DUTY_CYCLE-duty) / freq / MAX_DUTY_CYCLE );
   t[1]=nsToClocks((duty)/freq/MAX_DUTY_CYCLE);
   v[0]=Vdc*ADC_RES/VDD-Vpp*ADC_RES/VDD/2;
   v[1]=Vdc*ADC_RES/VDD+Vpp*ADC_RES/VDD/2;
}
void gen_sin(uint32_t *t, uint32_t *v, uint16_t Vpp, uint16_t Vdc, uint32_t freq) {
   uint16_t i;
   for(i=0;i<SIN_SIZE;i++){
      t[i]=nsToClocks(SIN_DELAYS[i]/freq);
      v[i]=SIN_OUTPUT[i]*Vpp/VDD+(Vdc-VDD/2);  //scale and add offset
   }
}

void gen_tri(uint32_t *t, uint32_t *v, uint16_t Vpp, uint16_t Vdc, uint32_t freq, uint16_t duty) {
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
uint16_t voltages[2];
uint32_t times[2];

void main(void)
{
   uint16_t data;
   uint32_t i;
   WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

   setupDAC();

   __enable_irq();

   NVIC->ISER[0] = 1 << ( (EUSCIB0_IRQn) & 31);
   
   while(1) {
      /* while( !(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG) ); */
      for(data=0; data < 5096; data++) {
	 setDAC(data);
	 /* FOR DELAY ONLY, can remove later */
	 /* for(i=0; i <200; i++); */
      }
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


void a6SquareWave() {
   gen_square(times, voltages, 2 , 1, 50, 500);

}

void EUSCIB0_IRQHandler(void) {

   volatile uint8_t RXData;

   if(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG) {
      RXData = EUSCI_B0->RXBUF;
   }
}
