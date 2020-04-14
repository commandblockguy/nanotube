#ifndef ARCH_CC_H
#define ARCH_CC_H

#include <stdint.h>
#include <debug.h>

//todo: remove
void mainlog(const char *);

#define LWIP_HAVE_INT64 0
#define LWIP_NO_INTTYPES_H 1
#define LWIP_PLATFORM_DIAG(x) //do {dbg_sprintf x;} while(0)
#define LWIP_PLATFORM_ASSERT(x) // do {sprintf(dbgout, "Assertion \"%s\" failed at line %d in %s\n", \
                                     x, __LINE__, __FILE__); fflush(NULL); abort();} while(0)
#define LWIP_RAND() ((u32_t)rand())

// who knows why
#undef EDOM
#undef ERANGE

#endif //ARCH_CC_H
