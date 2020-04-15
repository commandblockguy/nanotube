#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <fileioc.h>
#include <keypadc.h>

#include <debug.h>

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
        custom_printf("Packet %u received %u bytes.", pnum, size);
	} else {
        custom_printf("Packet %u sending %u bytes.", pnum, size);
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
#if LOG_TIMESTAMPS
    uint24_t time = timer_3_Counter / 33;
	char time_string[12];
	sprintf(time_string, "[%8u] ", time);
#endif
	//dbg_sprintf(dbgout, "%s%s\n", time_string, str);
#ifdef GRAPHICS
	fontlib_DrawString(str);
	fontlib_DrawString("\n");
#endif
	if(!pos) return;
	if(offset + length < LOG_SIZE
#if LOG_TIMESTAMPS
	- 11
#endif
	   - 2) {
#if LOG_TIMESTAMPS
        flash_write(&pos[offset], time_string, 11);
        offset += 11;
#endif
		flash_write(&pos[offset], str, length);
		offset += length;
        if(str[length - 1] != '\n') {
            flash_write(&pos[offset], "\n", 1);
            offset++;
        }
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
