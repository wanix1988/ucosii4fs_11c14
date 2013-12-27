#ifndef __COLLECT_DATA_H
#define __COLLECT_DATA_H

void collect_data(uint8_t *temp, uint8_t *hum,uint8_t *light);
void send_data(uint8_t *data, uint8_t len) ;
uint32_t adc_ret(uint8_t *adc);
void acc_ret(uint8_t *buffer);

#endif
