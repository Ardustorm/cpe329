#include "msp.h"
#include "myLibs/delay.h"


/* P3 */
#define RS BIT5
#define RW BIT6
#define EN BIT7
/* P4 : upper nyble */

#define FREQ_CUR FREQ_3MHz


#define SET_LOC      0x80

#define FUNCTION_SET    0x2C   /* 4-bit mode, 2 line, 5x8 */
#define DISP_ON      0x0f      /* Disp on, Curs on, blink on */
#define CLEAR_DISP      0x01
#define ENTRY_MODE      0x06	/* increment mode on, */
#define HOME_DISP      0x02

/* TEST used to toggle led pin to help with debug and logic analyser capture */
#define TEST    P1->OUT &= ~BIT0; delay_ms(1, FREQ_CUR); \
   P1->OUT |= BIT0; delay_ms(1, FREQ_CUR); P1->OUT &= ~BIT0;
/* #define TESTS(x) for(int y=x ; y > 0; y--) TEST; */

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
}
void LCD_Data(unsigned char data){
   P3->OUT |= EN | RS;		/* set enable and Reg select */
   P3->OUT &= ~RW;
   LCD_nybble(data>>4 );
   LCD_nybble(data);
   
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
