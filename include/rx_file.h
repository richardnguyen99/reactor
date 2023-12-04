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

#ifndef __RX_FILE_H__
#define __RX_FILE_H__ 1

#include <rx_config.h>
#include <rx_core.h>

/* File structure that reactor uses to work with files
 */
struct rx_file
{
    /* File descriptor of an opening file */
    int fd;

    /* Flags that are used to open `fd` */
    int flags;

    /* Absolute path to the directory that contains the opening file */
    char *path;

    /* File name without extension */
    char *name;

    /* Extension of the opening file */
    char *ext;

    /* MIME type of the extension respectively to HTTP standard */
    rx_http_mime_t mime;

    /* Size of the opening file */
    size_t size;

    /* Last modified time of the opening file

       This field is used to check if the file has been modified since
     */
    struct timespec mod;
};

/* Get MIME type, a value in `enum rx_http_mime`, from the given `file`

   The MIME type is determined by the extension of the file name, which appears
   after the last dot in the file name. If the file name does not have any dot,
   the MIME type is `RX_MIME_UNKNOWN`.

   For example:

   ```txt
   /usr/share/nginx/html/index.html -> RX_MIME_HTML (ext: "html")
   /usr/share/nginx/html/style.css  -> RX_MIME_CSS (ext: "css")
   /usr/share/nginx/html/script.js  -> RX_MIME_JS (ext: "js")
   ```
 */
rx_http_mime_t
rx_file_mime(const char *file, size_t len);

/* GET a string presentation of the given `mime`

   The MIME string follows the standard of RFC 1341 and RFC 2045. For example:

   ```txt
    RX_HTTP_MIME_HTML -> "text/html;charset=utf-8"
    RX_HTTP_MIME_CSS  -> "text/css;charset=utf-8"
    RX_HTTP_MIME_JSON -> "application/json;charset=utf-8"
   ```
 */
const char *
rx_file_strmime(rx_http_mime_t mime);

/* Open a file in `path` with `flags` and store the information in `fstruct`
 */
int
rx_file_open(struct rx_file *fstruct, const char *path, int flags);

/* Close the opening file and free the associated memory in `fstruct`
 */
int
rx_file_close(struct rx_file *fstruct);

#endif /* __RX_FILE_H__ */
