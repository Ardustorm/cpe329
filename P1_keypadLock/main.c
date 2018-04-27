#include "msp.h"
#include "myLibs/lcd.h"
#include "myLibs/delay.h"
#include "myLibs/keypad.h"

/**
 * Written by Luke Thompson and John Thomsen
 */

#include "password.h"


void main(void)
{
   WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
   int password_entered[PASS_LEN];
   
   LCD_Init();

   keypadInit();

   displayLocked();
   


   int i = 0;
   while(1) {


      password_entered[i] = getKeyCombo();
      Write_char_LCD(26+i, '*');

#ifdef DEBUG
      int j=0;
      for(j=13; j >=0; j--) {
	 if(password_entered[i] & 0x1 << j) {
	    Write_char_LCD(16 + 13-j, '1' );
	 } else {
	    Write_char_LCD(16 + 13-j, '0' );
	 }
      }
#endif
      
      i++;
      if( password_entered[i-1] == STAR) {
	 displayLocked();
	 i=0;
      }

      if( password_entered[i-1] == POUND) {
	 i-=2;
	 i = i>0? i : 0;
	 Write_char_LCD(26+i+1, ' ');
	 Write_char_LCD(26+i, ' ');

      }
      

      
      if(i == 4) {		/* Check password */
	 i=0;
	 Clear_LCD();
	 Write_str_LCD( 0, "Validating");
	 delay_ms(500,FREQ_3MHz);

	 if(checkPassword( password_entered )) {
	    Clear_LCD();
	    Write_str_LCD( 0, "HELLO WORLD");

	    getKeyCombo();	/* wait till keypressed */
	    displayLocked();	    
	    
	 } else {
	    Clear_LCD();
	    Write_str_LCD( 0, "Key Incorrect");
	    delay_ms(1500,FREQ_3MHz);
	    displayLocked();
	 }
      }

   
      delay_ms(50,FREQ_3MHz);
      
   }
}


int checkPassword(int pwd[]){
   int i;
   for(i = 0; i < PASS_LEN; i++) {
      if( pwd[i] != PASSWORD_KEY[i]) {
	 return 0;
      }
   }
   return 1;
}
      

void displayLocked(){
   Clear_LCD();
   Write_str_LCD( 0, "LOCKED");
   Write_str_LCD( 16, "ENTER KEY");
}
