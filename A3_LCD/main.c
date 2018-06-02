#include "msp.h"
#include "myLibs/delay.h"
#include "myLibs/lcd.h"

#define FREQ_CUR FREQ_3MHz

/* TEST used to toggle led pin to help with debug and logic analyser capture */
#define TEST    P1->OUT &= ~BIT0; delay_ms(1, FREQ_CUR); \
   P1->OUT |= BIT0; delay_ms(1, FREQ_CUR); P1->OUT &= ~BIT0;
/* #define TESTS(x) for(int y=x ; y > 0; y--) TEST; */



void main(void)
{
   WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
   P1->DIR |= BIT0;

   TEST;   
   LCD_Init();

   /* Write_char_LCD(0,'A'); */
   

   Write_str_LCD(4, "--> Hello World <--");
   delay_ms(1000, FREQ_CUR);

   TEST;
   
   int i;
   for(i = 0; i < 32; i++) {
      Clear_LCD();
      Write_char_LCD( i-1, 0xfd);
      Write_char_LCD( i, 0x7e);
      delay_ms(200, FREQ_CUR);
   }
   for( ; i >= 0; i--) {
      Clear_LCD();
      Write_char_LCD(i,0x7f);
      Write_char_LCD( i+1, '-');
      delay_ms(200, FREQ_CUR);
   }
   
   P1->OUT |= BIT0;
   delay_ms(50, FREQ_CUR);
   Clear_LCD();
   TEST; TEST;
}
