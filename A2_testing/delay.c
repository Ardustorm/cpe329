
void delay_ms(int time, int freq) {
   /* delays requested time in milliseconds using a busy loop */
   int i;
   /* for(i= time*(freq/10000); i > 0; i--) */
   for(i= time*(freq/9900); i > 0; i--)
      ;
}

void delay_us(int time, int freq) {
   /* delays requested time in microseconds using a busy loop */
   int i;
   for(i= time*(freq/11000)/1000; i > 0; i--)
      ;
}

