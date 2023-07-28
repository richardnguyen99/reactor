#include "request.h"
#include "response.h"

struct method_handler
{
    void (*get)(struct request *req, struct response *res);
    void (*post)(struct request *req, struct response *res);
    void (*put)(struct request *req, struct response *res);
    void (*delete)(struct request *req, struct response *res);
    void (*head)(struct request *req, struct response *res);
};

struct __route
{
    const char *const endpoint;
    const char *const resource;

    struct method_handler handler;
    const int status;
};

/* Router table for routing endpoints in the server. This table needs to be
initializes so that the server knows how to handling routing such as finding
resources and fulfilling method request. */
extern const struct __route router_table[];

struct __route
route_get_handler(const char *endpoint);
