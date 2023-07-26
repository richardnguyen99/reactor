#include "http.h"

int
_check_with_static_folder(const char *path)
{
    // This doesn't actually open the file, it just checks if it exists.
    int status = access(path, F_OK);

    if (status == -1)
        return ERROR;

    return SUCCESS;
}

int
_default_header_handler(const char *value)
{
    if (value == NULL)
        return ERROR;

    if (strlen(value) == 0)
        return ERROR;

    return SUCCESS;
}
