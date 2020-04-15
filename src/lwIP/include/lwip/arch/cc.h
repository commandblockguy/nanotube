#ifndef ARCH_CC_H
#define ARCH_CC_H

#include <stdint.h>
#include <debug.h>
#include <stdlib.h>

#include "../../../../log.h"

#define LWIP_HAVE_INT64 0
#define LWIP_NO_INTTYPES_H 1
#define LWIP_PLATFORM_DIAG(x) do {custom_printf x;} while(0)
#define LWIP_PLATFORM_ASSERT(x) do {custom_printf("Assertion \"%s\" failed at line %d in %s\n", \
                                     x, __LINE__, __FILE__); fflush(NULL); abort();} while(0)
#define LWIP_RAND() ((u32_t)rand())

#define U8_F "u"
#define U16_F "u"
#define U32_F "u"
#define S8_F "i"
#define S16_F "i"
#define S32_F "i"
#define X8_F "02x"
#define X16_F "04x"
#define X32_F "08x"

// who knows why
#undef EDOM
#undef ERANGE

#endif //ARCH_CC_H
