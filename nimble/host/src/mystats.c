

#include "string.h"
#include "host/mystats.h"

mystats_t mystats = { 0 };

void mystats_clear(void)
{
    memset(&mystats, 0, sizeof(mystats));
}

void mystats_get(mystats_t *out)
{
    memcpy(out, &mystats, sizeof(mystats));
}
