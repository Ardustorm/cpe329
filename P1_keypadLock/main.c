#include "msp.h"
#include "myLibs/lcd.h"
#include "myLibs/delay.h"
#include "myLibs/keypad.h"

/**
 * main.c
 */

/* int mapping[] = { '1', '4', '7', '*', */
/* 		  '2', '5', '8', '0', */
/* 		  '3', '6', '9', '#' }; */

#define ONE    (0x1 <<  0)
#define FOUR   (0x1 <<  1)
#define SEVEN  (0x1 <<  2)
#define STAR   (0x1 <<  3)

#define TWO    (0x1 <<  4)
#define FIVE   (0x1 <<  5)
#define EIGHT  (0x1 <<  6)
#define ZERO   (0x1 <<  7)

#define THREE  (0x1 <<  8)
#define SIX    (0x1 <<  9)
#define NINE   (0x1 << 10)
#define POUND  (0x1 << 11)

#define COMBO0 ONE | TWO
#define COMBO1 FIVE | SIX
#define COMBO2 SEVEN | EIGHT | NINE
#define COMBO3 EIGHT | ZERO

#define PASS_LEN 4

int PASSWORD_KEY[PASS_LEN] = { COMBO0, COMBO1, COMBO2, COMBO3};

void main(void)
{
   WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
   int password_entered[PASS_LEN];
   
   LCD_Init();
   keypadInit();
   Clear_LCD();
   displayLocked();
   
   int key = '-';


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
      

      
      if(i == 4) {		/* Check password */
	 i=0;
	 Clear_LCD();
	 Write_str_LCD( 0, "Validating");
	 delay_ms(500,FREQ_3MHz);

	 if(checkPassword( password_entered )) {
	    Clear_LCD();
	    Write_str_LCD( 0, "HELLO WORLD");
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
