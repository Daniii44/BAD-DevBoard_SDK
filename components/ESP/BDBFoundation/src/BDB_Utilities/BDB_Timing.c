#include "BDB_Timing.h"
#include "esp_timer.h"
#include "esp32s3/rom/ets_sys.h"

int64_t BDB_getMillis()
{
    return esp_timer_get_time()/1000;
}
int64_t BDB_getMicros()
{
    return esp_timer_get_time();
}

void BDB_delayMillis(uint32_t ms)
{
    ets_delay_us(ms*1000);
}
void BDB_delayMicroseconds(uint32_t us)
{
    ets_delay_us(us);
}