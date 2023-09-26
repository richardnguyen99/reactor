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

void
rx_log_debug(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf(ANSI_COLOR_GREEN "%-8s" ANSI_COLOR_RESET, "[INFO]");
    vprintf(fmt, args);
    va_end(args);
}

void
rx_log_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf(ANSI_COLOR_RED "%-8s" ANSI_COLOR_RESET, "[ERROR]");
    vprintf(fmt, args);
    va_end(args);
}

void
rx_log_warn(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf(ANSI_COLOR_YELLOW "%-8s" ANSI_COLOR_RESET, "[WARN]");
    vprintf(fmt, args);
    va_end(args);
}

void
rx_log(int level, int type, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    level = level << 2;

    switch (type)
    {
    case LOG_TYPE_INFO:
        printf(ANSI_COLOR_GREEN "%-8s%*.s" ANSI_COLOR_RESET, "[INFO]", level,
               " ");
        vprintf(fmt, args);
        va_end(args);
        break;
    case LOG_TYPE_WARN:
        printf(ANSI_COLOR_YELLOW "%-8s%*.s" ANSI_COLOR_RESET, "[WARN]", level,
               "");
        vprintf(fmt, args);
        va_end(args);
        break;
    case LOG_TYPE_ERROR:
        printf(ANSI_COLOR_RED "%-8s%*.s" ANSI_COLOR_RESET, "[ERROR]", level,
               "");
        vprintf(fmt, args);
        va_end(args);
        break;

    case LOG_TYPE_DEBUG:
    default:
#if defined(RX_DEBUG)
        printf(ANSI_COLOR_BLUE "%-8s%*.s" ANSI_COLOR_RESET, "[DEBUG]", level,
               "");
        vprintf(fmt, args);
        va_end(args);
#endif
        break;
    }
}
