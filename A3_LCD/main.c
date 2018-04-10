#include "msp.h"
#include "myLibs/delay.h"


/* P3 */
#define RS BIT5
#define RW BIT6
#define EN BIT7

/* P4 : upper nyble */

#define CLEAR_DISP      0x01
#define FUNCTION_SET    0x28   /* 4-bit mode, 2 line, 5x8 */

void LCD_Init(void){
   P3->DIR |= RS|RW|EN;
   P4->DIR |= 0xF0;


   LCD_Command(FUNCTION_SET);
   delay_ms(10, FREQ_3MHz);
   /* LCD_Command(0x20); */

   /* delay_ms(10, FREQ_3MHz); */
   /* LCD_Command(0x20); */
   
   /* delay_ms(10, FREQ_3MHz); */
   /* LCD_Command(0x20); */

   /* delay_ms(10, FREQ_3MHz); */
   /* LCD_Command(0x28); */

   /* delay_ms(10, FREQ_3MHz); */
   /* LCD_Command(0x0c); */

   /* delay_ms(10, FREQ_3MHz); */
   /* LCD_Command(0x06); */

   /* delay_ms(10, FREQ_3MHz); */
   /* LCD_Command(0x08); */
   
   
   
}

void LCD_Command(unsigned char CMD){
   P3->OUT &= ~(RS|RW|EN);
   P4->OUT &= 0x0F;
   P3->OUT |= EN;
   delay_us(100, FREQ_3MHz);


   /* TODO: check if right */
   P4->OUT |= CMD & 0xF0;
   delay_us(10, FREQ_3MHz);
   P4->OUT &= 0x0F;
   P3->OUT &= ~EN;
   
   P3->OUT |= EN;
   P4->OUT |= (CMD << 4);
   delay_us(10, FREQ_3MHz);
   
   P3->OUT &= ~EN;
   P4->OUT &= 0x0F;
   delay_us(1, FREQ_3MHz);
}
void LCD_Data(unsigned char data){
   P3->OUT &= ~(RS|RW|EN);
   P4->OUT &= 0x0F;
   P3->OUT |= EN | RS;
   delay_us(100, FREQ_3MHz);


   /* TODO: check if right */
   P4->OUT |= data & 0xF0;
   delay_us(10, FREQ_3MHz);
   P4->OUT &= 0x0F;
   P3->OUT &= ~(EN | RS);
   
   P3->OUT |= EN | RS;
   P4->OUT |= (data << 4);
   delay_us(10, FREQ_3MHz);
   
   P3->OUT &= ~(EN | RS);
   P4->OUT &= 0x0F;
   delay_us(1, FREQ_3MHz);
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
   /* delay_ms(100, FREQ_3MHz); */
   /* LCD_Command(0x0c); */
   /* P1->OUT |= BIT0; */

   /* LCD_Init(); */
   while(1) {
      /* LCD_Command(0xa5); */
      delay_ms(50, FREQ_3MHz);
      
      P1->OUT &= ~BIT0;
      LCD_Init();

      LCD_Data(0x41);
      /* LCD_Command(0x42); */
      /* LCD_Command(0xaa); */
      delay_ms(50, FREQ_3MHz);
      P1->OUT |= BIT0;
      delay_ms(50, FREQ_3MHz);
   }
}
