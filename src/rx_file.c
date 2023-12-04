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

static int
rx_file_ext2mime(const char *ext);

rx_http_mime_t
rx_file_mime(const char *file, size_t len)
{
    if (len == 0 || file == NULL)
        return RX_ERROR;

    char *ext     = strrchr(file, '.');
    size_t offset = ext - (char *)file;

    if (ext == NULL || offset >= len)
        return RX_HTTP_MIME_NONE;

    ext++;

    return rx_file_ext2mime(ext);
}

const char *
rx_file_mimestr(rx_http_mime_t mime)
{
    switch (mime)
    {
    case RX_HTTP_MIME_NONE:
        return RX_HTTP_MIME_NONE_STR;

    case RX_HTTP_MIME_ALL:
        return RX_HTTP_MIME_ALL_STR;

    case RX_HTTP_MIME_TEXT_ALL:
        return RX_HTTP_MIME_TEXT_ALL_STR;

    case RX_HTTP_MIME_TEXT_PLAIN:
        return RX_HTTP_MIME_TEXT_PLAIN_STR;

    case RX_HTTP_MIME_TEXT_HTML:
        return RX_HTTP_MIME_TEXT_HTML_STR;

    case RX_HTTP_MIME_TEXT_CSS:
        return RX_HTTP_MIME_TEXT_CSS_STR;

    case RX_HTTP_MIME_TEXT_JS:
        return RX_HTTP_MIME_TEXT_JS_STR;

    case RX_HTTP_MIME_TEXT_OCTET_STREAM:
        return RX_HTTP_MIME_TEXT_OCTET_STREAM_STR;

    case RX_HTTP_MIME_APPLICATION_XML:
        return RX_HTTP_MIME_APPLICATION_XML_STR;

    case RX_HTTP_MIME_APPLICATION_JSON:
        return RX_HTTP_MIME_APPLICATION_JSON_STR;

    case RX_HTTP_MIME_APPLICATION_XHTML:
        return RX_HTTP_MIME_APPLICATION_XHTML_STR;

    case RX_HTTP_MIME_APPLICATION_XFORM:
        return RX_HTTP_MIME_APPLICATION_XFORM_STR;

    case RX_HTTP_MIME_IMAGE_ALL:
        return RX_HTTP_MIME_IMAGE_ALL_STR;

    case RX_HTTP_MIME_IMAGE_ICO:
        return RX_HTTP_MIME_IMAGE_ICO_STR;

    case RX_HTTP_MIME_IMAGE_GIF:
        return RX_HTTP_MIME_IMAGE_GIF_STR;

    case RX_HTTP_MIME_IMAGE_JPEG:
        return RX_HTTP_MIME_IMAGE_JPEG_STR;

    case RX_HTTP_MIME_IMAGE_PNG:
        return RX_HTTP_MIME_IMAGE_PNG_STR;

    case RX_HTTP_MIME_IMAGE_SVG:
        return RX_HTTP_MIME_IMAGE_SVG_STR;

    default:
        return RX_HTTP_MIME_TEXT_OCTET_STREAM_STR;
    }
}

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
    fstruct->mime = rx_file_mime(fstruct->name, strlen(fstruct->name));
    fstruct->size = st.st_size;

    memset(&fstruct->mod, 0, sizeof(struct timespec));
    memcpy(&fstruct->mod, &st.st_mtim, sizeof(struct timespec));

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

static int
rx_file_ext2mime(const char *ext)
{
    if (strncmp(ext, "txt", 3) == 0)
        return RX_HTTP_MIME_TEXT_PLAIN;

    if (strncmp(ext, "html", 4) == 0)
        return RX_HTTP_MIME_TEXT_HTML;

    if (strncmp(ext, "css", 3) == 0)
        return RX_HTTP_MIME_TEXT_CSS;

    if (strncmp(ext, "js", 2) == 0)
        return RX_HTTP_MIME_TEXT_JS;

    if (strncmp(ext, "json", 4) == 0)
        return RX_HTTP_MIME_APPLICATION_JSON;

    if (strncmp(ext, "xml", 3) == 0)
        return RX_HTTP_MIME_APPLICATION_XML;

    if (strncmp(ext, "ico", 3) == 0)
        return RX_HTTP_MIME_IMAGE_ICO;

    if (strncmp(ext, "png", 3) == 0)
        return RX_HTTP_MIME_IMAGE_PNG;

    if (strncmp(ext, "jpg", 3) == 0 || strncmp(ext, "jpeg", 4) == 0)
        return RX_HTTP_MIME_IMAGE_JPEG;

    if (strncmp(ext, "svg", 3) == 0)
        return RX_HTTP_MIME_IMAGE_SVG;

    if (strncmp(ext, "gif", 3) == 0)
        return RX_HTTP_MIME_IMAGE_GIF;

    return RX_HTTP_MIME_TEXT_OCTET_STREAM;
}
