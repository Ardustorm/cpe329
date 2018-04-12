#ifndef LCD_H
#define LCD_H

/* P3 */
#define RS BIT5
#define RW BIT6
#define EN BIT7
/* P4 : upper nyble */



#define SET_LOC      0x80

#define FUNCTION_SET    0x2C   /* 4-bit mode, 2 line, 5x8 */
#define DISP_ON      0x0f      /* Disp on, Curs on, blink on */
#define CLEAR_DISP      0x01
#define ENTRY_MODE      0x06	/* increment mode on, */
#define HOME_DISP      0x02



void LCD_Init(void);
void LCD_nybble(unsigned char cmd);
void LCD_Command(unsigned char CMD);
void LCD_Data(unsigned char data);
void Clear_LCD();
void Home_LCD();
void Write_char_LCD(uint8_t loc, char c);
void Write_str_LCD(uint8_t loc, char *s);

#endif

