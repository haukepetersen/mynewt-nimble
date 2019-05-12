

#include <string.h>
#include "fmt.h"

#include "host/myfilter.h"

static uint8_t _list[MYFILTER_LISTLEN][6] = { 0 };
static unsigned _pos = 0;

int myfilter_drop(const uint8_t *addr)
{
    if (_pos == 0) {
        return 0;
    }

    for (unsigned i = 0; i < _pos; i++) {
        if (memcmp(_list[i], addr, 6) == 0) {
            return 0;
        }
    }

    return 1;
}

int myfilter_add(const uint8_t *addr)
{
    if (_pos == MYFILTER_LISTLEN) {
        return 1;
    }
    memcpy(_list[_pos++], addr, 6);
    return 0;
}

int myfilter_add_str(const char *addr_str)
{
    uint8_t addr[6];
    if (strlen(addr_str) != 12) {
        return 1;
    }
    for (unsigned i = 0; i < 6; i++) {
        uint32_t tmp = scn_u32_hex(&addr_str[i * 2], 2);
        addr[i] = (uint8_t)tmp;
    }
    return myfilter_add(addr);
}

void myfilter_clear(void)
{
    memset(_list, 9, sizeof(_list));
}
