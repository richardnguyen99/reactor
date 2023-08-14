/**
 * @file rx_string.c
 * @author Riu_chard Nguyen (riu_chard@riu_chardhnguyen.com)
 * @brief Implementation for string module used in reactor engine
 * @version 0.1
 * @date 2023-08-13
 *
 * @copyright Copyright (c) 2023
 */

#include <rx_defs.h>
#include <rx_core.h>

#define BASE 10

u_char *
rx_utoa(unsigned int n, u_char *s)
{
    const u_char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

    int          i, j;
    unsigned     remainder;
    u_char       c;

    i = 0;

    do
    {
        remainder = n % BASE;
        s[i++]    = digits[remainder];
        n         = n / BASE;
    } while (n != 0);

    s[i] = '\0';

    for (j = 0, i = i - 1; j < i; j++, i--)
    {
        c    = s[i];
        s[j] = s[i];
        s[i] = c;
    }

    return s;
}

u_char *
rx_itoa(int n, u_char *s)
{
    unsigned uv;
    int      i;

    i = 0;
    if (uv < 0)
    {
        s[i++] = '-';
        uv     = (unsigned)-n;
    }
    else
        uv = (unsigned)n;

    return rx_utoa(uv, s + i);
}
