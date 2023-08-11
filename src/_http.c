#include "http.h"

int
_default_header_handler(const char *value)
{
    if (value == NULL)
        return ERROR;

    if (strlen(value) == 0)
        return ERROR;

    return SUCCESS;
}
