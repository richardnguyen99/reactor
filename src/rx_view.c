/* MIT License
 *
 * Copyright (c) 2023 Richard H. Nguyen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <rx_config.h>
#include <rx_core.h>

struct rx_view rx_view_engine;

const char *const rx_view_base_client_error = "\
<!DOCTYPE html>\n\
<html lang=\"en\">\n\
<head>\n\
    <meta charset=\"UTF-8\">\n\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
    <meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">\n\
    <title>Document</title>\n\
</head>\n\
<body>\n\
    <h1>Client Error</h1>\n\
    <p>Sorry, but we can't process your request.</p>\n\
</body>\n\
</html>\n\
";

const char *const rx_view_base_server_error = "\
<!DOCTYPE html>\n\
<html lang=\"en\">\n\
<head>\n\
    <meta charset=\"UTF-8\">\n\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
    <meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">\n\
    <title>Document</title>\n\
</head>\n\
<body>\n\
    <h1>Server Error</h1>\n\
    <p>Sorry, but we can't process your request.</p>\n\
</body>\n\
</html>\n\
";

const char *const rx_view_header_template = "\
HTTP/1.1 %d %s\r\n\
Server: Reactor\r\n\
Content-Type: %s\r\n\
Content-Length: %d\r\n\
Date: %s\r\n\
Connection: %s\r\n\
\r\n\
";

static int
rx_view_map_file(struct rx_map_file *fmap, const char *path)
{
    int ret;
    char *data;

    memset(&fmap->file, 0, sizeof(struct rx_file));

    ret = rx_file_open(&fmap->file, path, O_RDONLY);

    if (ret != RX_OK)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_WARN, "%s: Failed to open file \'%s\'\n",
               __func__, path);

        return ret;
    }

    data =
        mmap(NULL, fmap->file.size, PROT_READ, MAP_PRIVATE, fmap->file.fd, 0);

    if (data == MAP_FAILED)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_WARN, "%s: Failed to map file \'%s\'\n",
               __func__, path);

        return RX_ERROR;
    }

    fmap->data = data;

    return ret;
}

int
rx_view_init()
{
    memset(&rx_view_engine, 0, sizeof(struct rx_view));

    memset(&rx_view_engine.base_template, 0, sizeof(struct rx_map_file));

    memset(&rx_view_engine.client_error_template, 0,
           sizeof(struct rx_map_file));

    memset(&rx_view_engine.server_error_template, 0,
           sizeof(struct rx_map_file));

    return RX_OK;
}

int
rx_view_load_template(const char *path)
{
    return rx_view_map_file(&rx_view_engine.base_template, path);
}

int
rx_view_load_4xx(const char *path)
{
    return rx_view_map_file(&rx_view_engine.client_error_template, path);
}

int
rx_view_load_5xx(const char *path)
{
    return rx_view_map_file(&rx_view_engine.server_error_template, path);
}

void
rx_view_destroy()
{
    if (rx_view_engine.base_template.data != NULL)
    {
        munmap(rx_view_engine.base_template.data,
               rx_view_engine.base_template.file.size);
    }

    if (rx_view_engine.base_template.file.fd != 0)
    {
        rx_file_close(&rx_view_engine.base_template.file);
    }

    if (rx_view_engine.client_error_template.data != NULL)
    {
        munmap(rx_view_engine.client_error_template.data,
               rx_view_engine.client_error_template.file.size);
    }

    if (rx_view_engine.client_error_template.file.fd != 0)
    {
        rx_file_close(&rx_view_engine.client_error_template.file);
    }

    if (rx_view_engine.server_error_template.data != NULL)
    {
        munmap(rx_view_engine.server_error_template.data,
               rx_view_engine.server_error_template.file.size);
    }

    if (rx_view_engine.server_error_template.file.fd != 0)
    {
        rx_file_close(&rx_view_engine.server_error_template.file);
    }
}
