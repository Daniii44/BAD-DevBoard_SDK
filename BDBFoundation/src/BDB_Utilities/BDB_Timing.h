#ifndef _BDB_TIMING_H
#define _BDB_TIMING_H

#include "esp_types.h"

int64_t BDB_getMillis();
int64_t BDB_getMicros();

void BDB_delayMillis(uint32_t ms);
void BDB_delayMicroseconds(uint32_t us);

#endif // _BDB_TIMING_H