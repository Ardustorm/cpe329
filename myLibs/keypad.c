#include "msp.h"
#include "keypad.h"




int scan() {
   int buttons = 0;
   P5->OUT &= ~0x07;		/* clear outpus */
   P5->OUT |= 0x01;

   buttons |= (P2->IN & 0xF0) << 0;

   P5->OUT &= ~0x07;		/* clear outpus */
   P5->OUT |= 0x02;

   buttons |= (P2->IN & 0xF0) >> 4;

   P5->OUT &= ~0x07;		/* clear outpus */
   P5->OUT |= 0x04;

   buttons |= (P2->IN & 0xF0) << 4;

   return buttons;
}


void keypadInit() {
   /* P2.4 to P2.7 are pins B, G, F, D in order */
   /* P5.0 to P5.2 are the outputs */
   P5->DIR |= 0x07;	/* Output */
   P5->OUT &= ~0x07;	/* zero Output */
   
   P2->REN |= 0xF0;
   P2->OUT &= ~0xF0;		/* pulldown */
   
}


int getKey() {
   int ret;

   while( (ret=scanKeypad()) == -1) {
   }

   return ret;
}
   

int scanKeypad(){
   /* returns the character of the button pressed or -1 if nothing is pressed */
   uint16_t buttons = 0;
   int mapping[] = { '1', '4', '7', '*',
		       '2', '5', '8', '0',
		       '3', '6', '9', '#' };

   buttons = scan();
   
   if( buttons == 0) {
      return -1;
   }

   
   int i = 0;
   while( (buttons & 0x1) == 0) {
      buttons = buttons >> 1;
      i++;
   }

   return mapping[i];
   
}



int getKeyCombo(){
   /* returns a bit array (an int) that represents the key combo
      that was pressed. 
      
      Note:
      If the keypad does not have diodes, phantom keypresses 
      will be detected
   */

   uint16_t buttons = 0;
   uint16_t allButtons = 0;

   while( (buttons=scan()) == 0)
      ;
   
   while( (buttons=scan()) != 0) {
      allButtons |=buttons;
   } 
   
 
   return allButtons;
   
}

