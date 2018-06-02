#include "msp.h"
#include <math.h>
/* #define var_type int8_t */
#define var_type int32_t
/* #define var_type int64_t */
/* #define var_type float */
/* #define var_type double */
var_type TestFunction(var_type num);

int main(void) {
   var_type mainVar;
   WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
   P1->SEL1 &= ~BIT0; //set P1.0 as simple I/O
   P1->SEL0 &= ~BIT0;
   P1->DIR |= BIT0; //set P1.0 as output
   
   P2->SEL1 &= ~(BIT2 | BIT1 | BIT0); //set P2.0-2.2 as simple I/O
   P2->SEL0 &= ~(BIT2 | BIT1 | BIT0);
   P2->DIR |= (BIT2 | BIT1 | BIT0); //set P2.0-2.2 as output pins

   P2->OUT |= (BIT2 | BIT1 | BIT0); // turn on RGB LED
   mainVar = TestFunction(15); // test function for timing
   P2->OUT &= ~(BIT2 | BIT1 | BIT0); // turn off RGB LED
   while(1)
      // infinite loop to do nothing
      mainVar++; // increment mainVar to eliminate not used warning
}



var_type TestFunction(var_type num ) {
   var_type testVar;
   P1->OUT |= BIT0; // set P1.0 LED on
   /* { insert_function_here (ie testVar = num;) } */
   testVar=abs(num);
   P1->OUT &= ~BIT0; // set P1.0 LED off
   return testVar;
}
