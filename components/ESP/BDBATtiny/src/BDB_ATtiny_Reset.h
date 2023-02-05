#ifndef _BDB_ATTINY_RESET_H
#define _BDB_ATTINY_RESET_H

#define ATTINY_RESET_ACTIVE_LEVEL 1 // The actual level applied to the #RESET pin of the ATTiny is inverted by a mosfet

void BDB_ATtiny_InitReset();
void BDB_ATtiny_DeinitReset();

void BDB_ATtiny_ApplyReset();
void BDB_ATtiny_ReleaseReset();

void BDB_ATtiny_Reset();

#endif // _BDB_ATTINY_RESET_H