

/* TEST used to toggle led pin to help with debug and logic analyser capture */
#define TEST    P1->OUT &= ~BIT0; delay_ms(1, FREQ_CUR); \
   P1->OUT |= BIT0; delay_ms(1, FREQ_CUR); P1->OUT &= ~BIT0;
/* #define TESTS(x) for(int y=x ; y > 0; y--) TEST; */
