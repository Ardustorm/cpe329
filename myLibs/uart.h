#ifndef UART_H
#define UART_H
#include "msp.h"

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#define CLR_SCREEN "\x1B[2J"
#define INVIS_CURSOR"\x1B[?25l"
#define LOC(x,y) "\x1B["#x";"#y"f"



void uartInit();

/* IRQ handler that get '\n' terminated numbers */
void EUSCIA0_IRQHandler(void);

/* returns the number if the uart flag is set, otherwise -1.
   The function clears the flag and the number after each call.
 */
int getUartNum();

#endif
