#include "msp.h"
#include "myLibs/delay.h"
#include "myLibs/uart.h"
#include <math.h>
#define MAX(a,b) (a>b)? a:b
#define MIN(a,b) (a<b)? a:b

/* Locations of where each field is located in the buffer */
#define DCOFFSET_LOCATION 43
#define RMS_LOCATION 57
#define PP_LOCATION 74
#define FREQ_LOCATION 99
#define DC_LOCATION 124
#define BAR1_LOCATION 140
#define BAR2_LOCATION 384
#define BAR3_LOCATION 623
#define BARLEN 50

volatile uint32_t period = 95;
volatile uint32_t dcSum  = 0;
volatile uint32_t dcOffsetSum  = 0;
volatile uint64_t rmsSum = 0;
volatile uint16_t vMin   = 0;
volatile uint16_t vMax   = 0;
volatile uint32_t numOfSamples = 0;
volatile uint32_t numOfDCSamples = 0; /* used for 1ms timing */

/* values of last measurements, used for calculations */
volatile uint32_t dcOffsetSumLast  = 0;
volatile uint64_t rmsSumLast = 0;
volatile uint16_t vMinLast   = 0;
volatile uint16_t vMaxLast   = 0;
volatile uint32_t numOfSamplesLast = 0;
volatile uint32_t periodLast=0;

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
   P2->SEL0 |= BIT5;        // TA0.CCI2A input capture pin, second function
   P2->DIR &= ~BIT5;

   /* setup for 1ms timer for DC */
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

   /* enable interrupts for capture mode and 1ms timer */
   NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);
   NVIC->ISER[0] = 1 << ((TA0_N_IRQn) & 31);
}


// Timer A0_0 interrupt service routine
void TA0_0_IRQHandler(void) {	/* ISR for 1ms update of DC Values */
   // Clear the compare interrupt flag
   TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
   insertFloat(buf + DC_LOCATION,  dcSum/numOfDCSamples/4094.0 *3.3);

   int barLen = dcSum/numOfDCSamples/4094.0 * BARLEN;
   memset(buf + BAR3_LOCATION,         '#', barLen);
   memset(buf + BAR3_LOCATION+barLen, '-',  BARLEN - barLen);

   dcSum=0;
   numOfDCSamples=0;
   
   TIMER_A0->CCR[0] += 33;	/* increment timer by 1 ms */

   /* check if DC by seeing if AC has been updated in a while,
      if so, hide ac fields*/
   if(numOfSamples > 200000) {
      memset(buf+DCOFFSET_LOCATION, '-', 5);
      memset(buf+RMS_LOCATION, '-', 5);
      memset(buf+PP_LOCATION, '-', 5);
      memset(buf+FREQ_LOCATION, '-', 5);
      memset(buf + BAR1_LOCATION, '~', BARLEN);
      memset(buf + BAR2_LOCATION, '~', BARLEN);
   }
}


/* ISR for end of a period (capture mode) */
void TA0_N_IRQHandler(void) {
   static uint16_t lastClk=0;

   if(TIMER_A0->CCR[2] > lastClk) {
      period = TIMER_A0->CCR[2]-lastClk;
   }
   lastClk= TIMER_A0->CCR[2];
   
   numOfSamplesLast = numOfSamples;
   dcOffsetSumLast  = dcOffsetSum;
   rmsSumLast       = rmsSum; 
   vMaxLast         = vMax;     
   vMinLast         = vMin; 
   periodLast       = period;
   
   numOfSamples=0;
   dcOffsetSum=0;
   rmsSum = 0;
   vMax=0;
   vMin=4094;
   
   insertFloat(buf + PP_LOCATION, (vMaxLast-vMinLast)/4094.0*3.3);
         
   // Clear the interrupt flag
   TIMER_A0->CCTL[2] &= ~(TIMER_A_CCTLN_CCIFG);
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

// ADC14 interrupt service routine, runs when a sample is ready
void ADC14_IRQHandler(void) {
   /* updates sample and new sample flag */
   numOfSamples++;
   numOfDCSamples++;

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


   P1->DIR |= BIT0;
   
   int i;
   for(i = 0; i < 30000; i++);	/* delay while things settle */
   ADC14->CTL0 |= ADC14_CTL0_ENC | ADC14_CTL0_SC; /* start adc */
   /* 
      This method could use some improvement. The general principle is
      that the buffer is always being written to the screen so other
      functions can just memset or update the buffer without waiting for 
      the serial. This would be better implemented as a function
      so as to not need the location pound defines, or the weird placement
      of things as to avoid changing said pound defines.
    */
   
   buf = malloc(800);
   strcpy(buf,
	  LOC(3,20) "DC Average" LOC(3,42) "RMS"        LOC(3,62)"PP" /* ends at 31 */
	  LOC(4,20) "_#___ V" LOC(4,40) "_#___ Vrms" LOC(4,60)"_#___ Vpp"
	  LOC(3,5) "FREQ"     
	  LOC(4,4) "____  kHz"

	  LOC(16,5) "DC"     
	  LOC(17,4) "____  Vdc"

	  LOC(7,3)  "|!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!| DC Offset"
	  LOC(8,3)  "||         |       |        |        |       |    '|"
	  LOC(9,3)  "|0        0.6     1.2      1.8      2.4     3.0 3.3|"
	  LOC(10,3) "||         |       |        |        |       |    '|"
	  LOC(11,3) "|!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!| RMS"

	  LOC(6,3)  "|==================================================|"
	  LOC(12,3) "|==================================================|"

	  LOC(19,3) "|==================================================|"
	  LOC(20,3) "|!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!| DC"
	  LOC(21,3) "||         |       |        |        |       |    '|"
	  LOC(22,3) "|0        0.6     1.2      1.8      2.4     3.0 3.3|"
	  LOC(23,3) "|==================================================|"
	  
	  );
   setOutput(buf);		/* tells uart which string to print */


   float rmsAvg = 1;
   float periodAvg = 1;
   float dcOffsetAvg = 1;
   int barLen=0;
   while(1) {
      /* 
	 The += and /=2 etc is for exponential averaging to help
	 smooth out the numbers.
       */
      if(numOfSamplesLast) {	/* checks to make sure new info is avalible */
	 dcOffsetAvg += dcOffsetSumLast/numOfSamplesLast/4094.0 *3.3;
	 dcOffsetAvg/=2;
	 insertFloat(buf + DCOFFSET_LOCATION, dcOffsetAvg);

	 rmsAvg *= 7;
	 rmsAvg += sqrt(1.0*rmsSumLast/numOfSamplesLast)/4094.0*3.2;
	 rmsAvg/=8;
	 insertFloat(buf + RMS_LOCATION, rmsAvg );

	 periodAvg *= 7;
	 periodAvg += 32.768/periodLast;
	 periodAvg /=8;
	 insertFloat(buf +FREQ_LOCATION, periodAvg );

	 /* BAR GRAPHS */
	 barLen = MIN(dcOffsetSumLast/numOfSamplesLast/4094.0 * BARLEN, BARLEN);
	 memset(buf + BAR1_LOCATION,         '#', barLen);
	 memset(buf + BAR1_LOCATION+barLen, '-',  BARLEN - barLen);

	 barLen = MIN(rmsAvg *BARLEN/3.3, BARLEN);
	 memset(buf + BAR2_LOCATION,         '#', barLen);
	 memset(buf + BAR2_LOCATION+barLen, '-',  BARLEN - barLen);
	 int i = 0;
	 for(i=0; i< 100000; i++); /* short delay */
	 numOfSamplesLast=0;
      }
       
   }
}


