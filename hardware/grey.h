#ifndef __GREY_H
#define __GREY_H

typedef enum{
    channel_1 = 0u,
    channel_2,
    channel_3,
    channel_4,
    channel_5,
    channel_6,
    channel_7,
    channel_8,
    channel_ALL,
} CHANNEL_t;

void read_all_adc(uint16_t out_buf[8]);


#endif
