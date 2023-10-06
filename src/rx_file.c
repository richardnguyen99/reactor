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

/* Returns malloc'd string to represent the HTTP MIME type (RFC 6838)
supported in reactor */
static char *
rx_file_get_mime(const char *ext);

int
rx_file_open(struct rx_file *fstruct, const char *path, int flags)
{
    int fd = open(path, flags);
    struct stat st;

    if (fd < 0)
        return RX_FATAL_WITH_ERROR;

    fstruct->fd = fd;

    if (fstat(fd, &st) < 0)
    {
        close(fd);
        return RX_FATAL_WITH_ERROR;
    }

    fstruct->path = strdup(path);
    if (fstruct->path == NULL)
    {
        close(fd);
        return RX_ALLOC_FAILED;
    }

    fstruct->flags = flags;
    fstruct->name  = basename(fstruct->path);
    fstruct->ext   = strrchr(fstruct->name, '.');

    if (fstruct->ext != NULL)
    {
        // The current pointer is at the dot. Advance the point to get the ext
        fstruct->ext++;
    }

    // TODO: Add support for file types
    fstruct->mime = rx_file_get_mime(fstruct->ext);
    fstruct->size = st.st_size;

    return RX_OK;
}

int
rx_file_close(struct rx_file *fstruct)
{
    if (fstruct->path != NULL)
    {
        free(fstruct->path);
    }

    if (fstruct->fd >= 0)
    {
        close(fstruct->fd);
    }

    return RX_OK;
}

static char *
rx_file_get_mime(const char *ext)
{
    if (ext == NULL)
    {
        return "text/plain";
    }

    if (strcmp(ext, "html") == 0)
    {
        return "text/html";
    }

    if (strcmp(ext, "css") == 0)
    {
        return "text/css";
    }

    if (strcmp(ext, "js") == 0)
    {
        return "text/javascript";
    }

    if (strcmp(ext, "json") == 0)
    {
        return "application/json";
    }

    if (strcmp(ext, "xml") == 0)
    {
        return "application/xml";
    }

    if (strcmp(ext, "ico") == 0)
    {
        return "image/x-icon";
    }

    if (strcmp(ext, "gif") == 0)
    {
        return "image/gif";
    }

    return "text/octet-stream";
}
