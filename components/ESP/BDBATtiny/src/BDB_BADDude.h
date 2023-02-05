#ifndef _BDB_BADDUDE_H
#define _BDB_BADDUDE_H

typedef struct
{
    int (*getCustomProgramCount)();
    const char *(*getCustomProgramTitle)(int);
    void (*runCustomProgram)(int);
} baddude_callback_t;

void BADDude_start(baddude_callback_t *baddude_callback);

#endif // _BDB_BADDUDE_H