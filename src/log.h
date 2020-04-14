#ifndef NANOTUBE_LOG_H
#define NANOTUBE_LOG_H

#include <stddef.h>
#include <stdbool.h>

#define CAPTURE_PACKETS 0
#define LOG_PACKETS 1
#define LOG_NAME "ETHLOG"
#define LOG_SIZE 64000

bool log_init(void);

#if LOG_PACKETS || CAPTURE_PACKETS
void log_packet(void *data, size_t size, bool receive);
#else
#define log_packet(data, size, receive)
#endif

void mainlog(const char* str);

void delete_old_logs(void);

void flash_write(void *flash_loc, const void *data, size_t size);

#endif //NANOTUBE_LOG_H
