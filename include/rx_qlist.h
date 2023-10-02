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

#ifndef __RX_QLIST_H__
#define __RX_QLIST_H__ 1

#include <rx_config.h>
#include <rx_core.h>

#define RX_QLIST_VALUE_SIZE 64

struct rx_qlist_node
{
    char value[RX_QLIST_VALUE_SIZE];
    float weight;

    struct rx_qlist_node *prev;
    struct rx_qlist_node *next;
};

struct rx_qlist
{
    struct rx_qlist_node *head;
    struct rx_qlist_node *tail;
    size_t size;
};

int
rx_qlist_create(struct rx_qlist *list);

void
rx_qlist_destroy(struct rx_qlist *list);

int
rx_qlist_add(struct rx_qlist *list, const char *value, size_t len,
             float weight);

#endif /* __RX_QLIST_H__ */
