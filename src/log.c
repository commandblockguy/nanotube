#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <fileioc.h>
#include <keypadc.h>

#include <debug.h>
#include <fontlibc.h>

#include "log.h"

char *pos;
uint24_t offset;

char logbuf[1024];

bool log_init(void) {
	ti_var_t slot;

	ti_CloseAll();
	delete_old_logs();

	dbg_sprintf(dbgout, "initting logs\n");

	/* We can't make a log file for ourselves for lack of space */
	/* A helper program (LOGINIT) will create one for us until I */
	/* work out the space / size issues */

	slot = ti_Open(LOG_NAME, "r");
	if(!slot) {
	    /* Log file does not exist */
		return false;
	}
	pos = ti_GetDataPtr(slot);
	offset = 0;

	if(*pos != (char)0xFF) {
        /* Log already has data */
        ti_Close(slot);
        return false;
	}

	ti_Close(slot);
	return true;
}

#if LOG_PACKETS || CAPTURE_PACKETS
void log_packet(void *data, size_t size, bool receive) {

	static uint24_t pnum = 0;

#if CAPTURE_PACKETS
	ti_var_t slot;
	char filename[9];

	sprintf(filename, "P%05u%cX", pnum, receive ? 'R' : 'T');
	slot = ti_Open(filename, "w");
	if(slot) {
		ti_Write(data, size, 1, slot);
		ti_SetArchiveStatus(true, slot);
		ti_Close(slot);
	}
#endif

#if LOG_PACKETS
	if(receive) {
        custom_printf("Packet %u received %u bytes.\n", pnum, size);
	} else {
        custom_printf("Packet %u sending %u bytes.\n", pnum, size);
	}
#endif
	pnum++;
}
#endif

void custom_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsprintf(logbuf, format, args);
    mainlog(logbuf);
}

void mainlog(const char *str) {
	size_t length = strlen(str);
	if(!pos) return;
	if(offset + length < LOG_SIZE - 12) {
#if LOG_TIMESTAMPS
	    if(pos[offset - 1] == '\n') {
            uint24_t time = timer_3_Counter / 33;
            char time_string[12];
            sprintf(time_string, "[%8u] ", time);
            //dbg_sprintf(dbgout, "%s", time_string);
            flash_write(&pos[offset], time_string, 11);
#if GRAPHICS
            fontlib_DrawString(time_string);
#endif
            offset += 11;
        }
#endif
        //dbg_sprintf(dbgout, "%s", str);
		flash_write(&pos[offset], str, length);
#if GRAPHICS
        fontlib_DrawString(str);
#endif
		offset += length;
	}
}

void delete_old_logs(void) {
	char* filename = NULL;
	void* search_pos = NULL;

	while((filename = ti_Detect(&search_pos, ""))) {
		if(filename[0] == 'P' && (filename[6] == 'T' || filename[6] == 'R') && filename[7] == 'X') {
			dbg_sprintf(dbgout, "deleting %s\n", filename);
			ti_Delete(filename);
		} else {
			dbg_sprintf(dbgout, "ignoring %s\n", filename);
		}
	}
}
