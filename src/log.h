#ifndef NANOTUBE_LOG_H
#define NANOTUBE_LOG_H

#include <stddef.h>
#include <stdbool.h>

#define CAPTURE_PACKETS 0
#define LOG_PACKETS 0
#define LOG_NAME "ETHLOG"
#define LOG_SIZE 64000
#define LOG_TIMESTAMPS 1
#define GRAPHICS 1

extern char logbuf[1024];

bool log_init(void);

#if LOG_PACKETS || CAPTURE_PACKETS
void log_packet(void *data, size_t size, bool receive);
#else
#define log_packet(data, size, receive)
#endif

#define LOG_LINE() custom_printf("line %u\n", __LINE__)

void custom_printf(const char* arg, ...);

void mainlog(const char* str);

void delete_old_logs(void);

void flash_write(void *flash_loc, const void *data, size_t size);

#endif //NANOTUBE_LOG_H
