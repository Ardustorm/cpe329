#ifndef KEYPAD_H
#define KEYPAD_H


void keypadInit();
/* P2.4 to P2.7 are pins B, G, F, D in order */
/* P5.0 to P5.2 are the outputs */


/* returns the character of the button pressed or -1 if nothing is pressed */
int scanKeypad();

/* Blocks untill a key is pressed then returns the key */
int getKey();


/* waits untill a key is pressed, then keeps track of all other keys pressed
   untill all keys are let go (NOTE: will have phantom keys if keypad doesn't have diodes*/
int getKeyCombo();

#endif
