#include "route.h"

struct __route
route_get_handler(const char *endpoint)
{
    for (int i = 0; router_table[i].endpoint != NULL; i++)
    {
        if (strcmp(router_table[i].endpoint, endpoint) == 0)
            return (struct __route){.endpoint = router_table[i].endpoint,
                                    .resource = router_table[i].resource,
                                    .handler  = router_table[i].handler,
                                    .status   = router_table[i].status};
    }

    return (struct __route){NULL, NULL, {0}, -1};
}
