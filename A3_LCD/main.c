#include "msp.h"
#include "myLibs/delay.h"


/* P3 */
#define RS BIT5
#define RW BIT6
#define EN BIT7
/* P4 : upper nyble */

#define FREQ_CUR FREQ_3MHz


#define FUNCTION_SET    0x2C   /* 4-bit mode, 2 line, 5x8 */
#define DISP_ON      0x0f      /* Disp on, Curs on, blink on */
#define CLEAR_DISP      0x01
#define ENTRY_MODE      0x07	/* increment mode on, autoshift on */



#define TEST    P1->OUT &= ~BIT0; delay_ms(1, FREQ_CUR); \
   P1->OUT |= BIT0; delay_ms(1, FREQ_CUR); P1->OUT &= ~BIT0;
/* #define TESTS(x) for(int y=x ; y > 0; y--) TEST; */

void LCD_Init(void){
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
   delay_ms(5, FREQ_CUR);

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

   LCD_nybble(CMD>>4 );
   LCD_nybble(CMD);
}
void LCD_Data(unsigned char data){
   P3->OUT |= EN | RS;
   P3->OUT &= ~RW;
   LCD_nybble(data>>4 );
   LCD_nybble(data);
}





/**
 * main.c
 */
void main(void)
{
   WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
   P1->DIR |= BIT0;
   P4->DIR |= 0xf0;
   /* P1->OUT &= ~BIT0; */
   /* LCD_Init(); */
   /* delay_ms(100, FREQ_CUR); */
   /* LCD_Command(0x0c); */
   /* P1->OUT |= BIT0; */

   TEST;
   
   LCD_Init();


   LCD_Command(0x8F);
   LCD_Data(0x41);
   LCD_Data(0x42);
   LCD_Data(0x43);
   
   delay_ms(50, FREQ_CUR);
   P1->OUT |= BIT0;
   delay_ms(50, FREQ_CUR);

   TEST; TEST;
}
