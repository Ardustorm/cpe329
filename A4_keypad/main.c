#include "msp.h"
#include "myLibs/lcd.h"
#include "myLibs/delay.h"

#include "keypad.h"
/**
 * main.c
 */



void main(void)
{
   WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

   LCD_Init();
   keypadInit();

   int key = '-';
   int i = 0;
   while(1) {

      Write_char_LCD( 0, key);
      key = getKey();
      delay_ms(100,FREQ_3MHz);
   /*    if( (key = scanKeypad()) != -1) { */
   /* 	 Write_char_LCD( i, key); */
   /* 	 i++; */
   /*    } */
   /*    if(i == 15) { */
   /* 	 Clear_LCD(); */
   /* 	 i=0; */
   /*    } */
   /*    delay_ms(200, FREQ_3MHz); */
   }

	
}
