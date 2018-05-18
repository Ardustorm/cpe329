#include "msp.h"
#include "myLibs/delay.h"
#include "lcd.h"


#define FREQ_CUR FREQ_3MHz
void LCD_Init(void){
   /* 
      Nybble mode can be confusing. To get started, here are the
      nybbles that must be sent:
      0x2, 0x2, 0xC (function set, notice upper nyble sent twice)
      0x0, 0xF (turn display on)
      0x0, 0x1 (clear the display)
      0x0, 0x6 (set entry mode) (might be optional)

    */
   
   P3->DIR |= RS|RW|EN;
   P4->DIR |= 0xF0;

   delay_ms(50, FREQ_CUR);

   
   P3->OUT &= ~(RS|RW|EN);
   LCD_nybble(FUNCTION_SET >>4);
   LCD_Command(FUNCTION_SET);
   delay_us(50, FREQ_CUR);


   LCD_Command(DISP_ON);
   delay_us(50, FREQ_CUR);
   
   LCD_Command(CLEAR_DISP);

   LCD_Command(ENTRY_MODE);
   delay_ms(5, FREQ_CUR);
   
   
   
}


void LCD_nybble(unsigned char cmd) {
   /* sends lower nybble of cmd */
   P4->OUT &= 0x0F;
   P3->OUT |= EN;		/* set enable */

   P4->OUT |= (cmd << 4);	/* put out lower bits */
   delay_us(10, FREQ_CUR);
   
   P3->OUT &= ~EN;		/* clear enable */
   P4->OUT &= 0x0F;		/* clear output bits */
   delay_us(1, FREQ_CUR);

}
   

void LCD_Command(unsigned char CMD){
   P3->OUT &= ~(RS|RW|EN);

   LCD_nybble(CMD>>4 );		/* upper nybble */
   LCD_nybble(CMD);		/* lower nybble */
   delay_us(100, FREQ_CUR);
}
void LCD_Data(unsigned char data){
   P3->OUT |= EN | RS;		/* set enable and Reg select */
   P3->OUT &= ~RW;
   LCD_nybble(data>>4 );
   LCD_nybble(data);
   delay_us(100, FREQ_CUR);   
   P3->OUT &= ~(EN | RS);
}


void Clear_LCD() {
   LCD_Command(CLEAR_DISP);
   delay_ms(10, FREQ_CUR);
}

void Home_LCD() {
   LCD_Command(HOME_DISP);
   delay_ms(1, FREQ_CUR);
}


void Write_char_LCD(uint8_t loc, char c) {
   /* location 0 to 31, and character, 16-31 are on line 2*/
   loc = loc < 16 ? loc : loc + 0x40 - 16;
   
   LCD_Command(SET_LOC | loc);
   LCD_Data(c);

}

void Write_str_LCD(uint8_t loc, char *s){
   /* writes to display, wrapping at boundry*/
   while(*s){
      Write_char_LCD(loc, *s);
      s++;
      loc++;
   }
}

