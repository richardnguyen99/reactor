#include "request.h"

void
request_free(struct request *request)
{
    if (request == NULL)
        return;

    free(request);

    return;
}
