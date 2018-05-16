#include "msp.h"
#include "myLibs/delay.h"
#include "myLibs/uart.h"
#include <math.h>
#define MAX(a,b) (a>b)? a:b
#define MIN(a,b) (a<b)? a:b
/**
 * main.c
 */

#define DCOFFSET_LOCATION 42
#define RMS_LOCATION 56
#define PP_LOCATION 73
#define FREQ_LOCATION 98
#define DC_LOCATION 121
#define BAR1_LOCATION 137
#define BARLEN 50
volatile uint32_t period = 95;
volatile uint32_t dcSum  = 0;
volatile uint32_t dcOffsetSum  = 0;
volatile uint64_t rmsSum = 0;
volatile uint16_t vMin   = 0;
volatile uint16_t vMax   = 0;
volatile uint32_t numOfSamples = 0;
volatile uint32_t numOfDCSamples = 0;
volatile char * buf;

void insertFloat(char * loc, float val) {
   /* currently, this function must take a float less than 10
      and will only print out 3 decimal places*/
   int i;

   for(i=4; i>0; i-- ) {

      *(loc++) = '0' + (int)(val);
      val -= (int)val;
      val *= 10;

      if(i==4) {		/* print decimal */
	 *(loc++) = '.';
      }
   }
}



void edgeTriggerInit() {
   P2->SEL0 |= BIT5;                       // TA0.CCI2A input capture pin, second function
   P2->DIR &= ~BIT5;

   /* setup for 1ms timer */
   TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
   TIMER_A0->CCR[0] = 32763;
   
   // Timer0_A3 Setup
   TIMER_A0->CCTL[2] = TIMER_A_CCTLN_CM_1 | // Capture rising edge,
      TIMER_A_CCTLN_CCIS_0 |          // Use CCI2A=ACLK,
      TIMER_A_CCTLN_CCIE |            // Enable capture interrupt
      TIMER_A_CCTLN_CAP |             // Enable capture mode,
      TIMER_A_CCTLN_SCS;              // Synchronous capture

   TIMER_A0->CTL |= TIMER_A_CTL_TASSEL_1 | // Use ACLK as clock source,
      TIMER_A_CTL_MC_2 |              // Start timer in continuous mode
      TIMER_A_CTL_CLR;                // clear TA0R

   NVIC->ISER[0] = 1 << ((TA0_N_IRQn) & 31);
}

void TA0_N_IRQHandler(void) {
   static uint16_t lastClk=0;

   if(TIMER_A0->CCTL[0] & TIMER_A_CCTLN_CCIFG) {
      insertFloat(buf + DC_LOCATION, dcSum/numOfDCSamples/4094.0 *3.3);
      dcSum=0;
      numOfDCSamples=0;

      TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
      TIMER_A0->CCR[0] += 33;
   } else  {

      if(TIMER_A0->CCR[2] > lastClk) {
	 period = TIMER_A0->CCR[2]-lastClk;
      }
      lastClk= TIMER_A0->CCR[2];

      insertFloat(buf + DCOFFSET_LOCATION,  dcOffsetSum/numOfSamples/4094.0 *3.3);
      insertFloat(buf + RMS_LOCATION, sqrt(rmsSum/numOfSamples)/4094.0*3.3);
      insertFloat(buf + PP_LOCATION, (vMax-vMin)/4094.0*3.3);


      /* BAR GRAPHS */
      int bar1Len = dcOffsetSum/numOfSamples/4094.0 * BARLEN;
      memset(buf + BAR1_LOCATION,         '#', bar1Len);
      memset(buf + BAR1_LOCATION+bar1Len, ' ',  BARLEN - bar1Len);


      /* memset(buf + BAR2_LOCATION,         '#', bar1Len); */
      /* memset(buf + BAR2_LOCATION+bar1Len, ' ',  BARLEN - bar1Len); */
      
      numOfSamples=0;
      dcOffsetSum=0;
      rmsSum = 0;
      vMax=0;
      vMin=4094;

      /* insertFloat(buf + DC_LOCATION,  dcSum/numOfSamples/4094.0 *3.3); */
      insertFloat(buf +FREQ_LOCATION, 32.768/period);
         
      // Clear the interrupt flag
      TIMER_A0->CCTL[2] &= ~(TIMER_A_CCTLN_CCIFG);
      
   }
}

void adcInit() {
   // GPIO Setup
   P5->SEL1 |= BIT4;                       // Configure P5.4 for ADC
   P5->SEL0 |= BIT4;


   // Enable ADC interrupt in NVIC module
   NVIC->ISER[0] = 1 << ((ADC14_IRQn) & 31);

   // Sampling time, S&H=16, ADC14 on
   ADC14->CTL0 = ADC14_CTL0_SHT0_2 | ADC14_CTL0_SHP | ADC14_CTL0_ON;
   ADC14->CTL1 = ADC14_CTL1_RES_2;         // Use sampling timer, 12-bit conversion results

   ADC14->MCTL[0] |= ADC14_MCTLN_INCH_1;   // A1 ADC input select; Vref=AVCC
   ADC14->IER0 |= ADC14_IER0_IE0;          // Enable ADC conv complete interrupt
}

// ADC14 interrupt service routine
void ADC14_IRQHandler(void) {
   /* updates sample and new sample flag */
   numOfSamples++;
   numOfDCSamples++;
   /* sample = ADC14->MEM[0]; */
   dcSum  += ADC14->MEM[0];
   dcOffsetSum  += ADC14->MEM[0];
   rmsSum += ADC14->MEM[0] * ADC14->MEM[0];
   vMin = MIN(vMin, ADC14->MEM[0]);
   vMax = MAX(vMax, ADC14->MEM[0]);
   ADC14->CTL0 |= ADC14_CTL0_ENC | ADC14_CTL0_SC;
 }


void main(void)
{
   WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
   set_DCO(FREQ_48MHz);
   uartInit();
   edgeTriggerInit();
   adcInit();
   
   int i;
   for(i = 0; i < 30000; i++);
   ADC14->CTL0 |= ADC14_CTL0_ENC | ADC14_CTL0_SC;
   buf = malloc(500);
   strcpy(buf,
	  LOC(5,22) "DC OFFSET" LOC(5,42) "RMS"        LOC(5,62)"PP" /* ends at 31 */
	  LOC(6,20) "_#___ V" LOC(6,40) "_#___ Vrms" LOC(6,60)"_#___ Vpp"
	  LOC(5,5) "FREQ"     
	  LOC(6,4) "____  kHz"

	  LOC(8,5) "DC"     
	  LOC(9,4) "____  Vdc"

	  LOC(12,4) "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
	  
	  );
   setOutput(buf);

   float f = 3.219;
   int arrayStart=0;
   while(1) {
      int i;
      for(i = 0; i < 5000; i++);
      /* insertFloat(buf + DC_LOCATION, dcSum/4094.0/numOfSamples *3.3); */
      /* numOfSamples=0; */
      /* dcSum=0; */
      /* insertFloat(buf + RMS_LOCATION, f); */
      /* insertFloat(buf + PP_LOCATION, f); */
       /* if (EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG) { */
       /* 	  EUSCI_A0->TXBUF = buf[arrayStart++]; */
       /* 	  if(buf[arrayStart] == '\0') { */
       /* 	     arrayStart = 0; */
       /* 	  } */
       /* } */
 
   }
}



   

