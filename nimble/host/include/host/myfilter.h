
#ifndef H_MYFILTER_
#define H_MYFILTER_

#include <stdio.h>
#include <stdint.h>

#include "xtimer.h"
#include "mutex.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MYFILTER_LISTLEN        (2U)

int myfilter_drop(const uint8_t *addr);

int myfilter_add(const uint8_t *addr);

int myfilter_add_str(const char *addr_str);

void myfilter_clear(void);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif
