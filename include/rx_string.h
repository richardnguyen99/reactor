/**
 * @file rx_string.h
 * @author Richar Nguyen (richard@richardnguyen.com)
 * @brief Header for string module
 * @version 0.1
 * @date 2023-08-13
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _RX_STRING_H_
#define _RX_STRING_H_ 1

#include <rx_defs.h>
#include <rx_core.h>

struct rx_string
{
    u_char *data; /* String pointer to the content */
    size_t  len;  /* String length */
};

struct rx_kv_pair
{
    rx_str_t key;   /* Key of the pair */
    rx_str_t value; /* Value of the pair */
};

#define rx_string(str)                                                         \
    {                                                                          \
        sizeof(str) - 1, (u_char *)str                                         \
    }
#define rx_null_string                                                         \
    {                                                                          \
        0, NULL                                                                \
    }

#define rx_string_set(str, text)                                               \
    do                                                                         \
    {                                                                          \
        (str)->len  = sizeof(text) - 1;                                        \
        (str)->data = (u_char *)text;                                          \
    } while (0)

#define rx_string_set_null(str)                                                \
    do                                                                         \
    {                                                                          \
        (str)->len  = 0;                                                       \
        (str)->data = NULL;                                                    \
    } while (0)

#define rx_strcmp(s1, s2)     strcmp((const char *)s1, (const char *)s2)
#define rx_strncmp(s1, s2, n) strncmp((const char *)s1, (const char *)s2, n)
#define rx_strstr(s1, s2)     strstr((const char *)s1, (const char *)s2)
#define rx_strlen(s)          strlen((const char *)s)
#define rx_strchr(s1, c)      strchr((const char *)s1, c)

#define rx_memset(buf, c, n)    (void)memset(buf, c, n)
#define rx_memzero(buf, n)      (void)memset(buf, 0, n)
#define rx_memcpy(dst, src, n)  (void)memcpy(dst, src, n)
#define rx_memmove(dst, src, n) (void)memmove(dst, src, n)

u_char *
rx_utoa(unsigned int n, u_char *s);

u_char *
rx_itoa(int n, u_char *s);

#endif /* _RX_STRING_H_ */
