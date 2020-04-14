#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fileioc.h>
#undef NDEBUG
#define DEBUG
#include <debug.h>

#define LOG_NAME "ETHLOG"
#define LOG_SIZE 64000

void main(void) {
    ti_var_t slot;

	ti_CloseAll();

	dbg_sprintf(dbgout, "initting logs\n");

	slot = ti_Open(LOG_NAME, "w+");
	if(!ti_Resize(LOG_SIZE, slot)) {
		dbg_sprintf(dbgout, "failed to resize, guess I'll die\n");
		return;
	}
	ti_Rewind(slot);
	memset(ti_GetDataPtr(slot), 0xFF, LOG_SIZE);
	ti_SetArchiveStatus(true, slot);
}
