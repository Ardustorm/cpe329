#ifndef DELAY_H
#define DELAY_H

#define FREQ_1_5MHz 1500000
#define FREQ_3MHz 3000000
#define FREQ_6MHz 6000000
#define FREQ_12MHz 12000000
#define FREQ_24MHz 24000000
#define FREQ_48MHz 48000000

void delay_us(int time, int freq);
void delay_ms(int time, int freq);

void set_DCO(int freq);

#endif
