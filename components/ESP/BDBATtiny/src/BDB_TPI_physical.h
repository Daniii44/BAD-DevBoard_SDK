#ifndef _TPI_PHYSICAL_H
#define _TPI_PHYSICAL_H

#include "esp_types.h"

void TPI_PHY_init();
void TPI_enter();
void TPI_exit(bool releaseReset);
void TPI_PHY_deinit(bool deinitReset);

void TPI_writeFrame(uint8_t data);
uint8_t TPI_readFrame();

void TPI_writeBit(uint8_t bit);
uint8_t TPI_readBit();

#endif // _TPI_PHYSICAL_H