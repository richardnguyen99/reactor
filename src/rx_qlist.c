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

int
rx_qlist_create(struct rx_qlist *list)
{
    struct rx_qlist_node *node = malloc(sizeof(struct rx_qlist_node));

    if (node == NULL)
        return RX_ALLOC_FAILED;

    list->tail = NULL;
    node->prev = NULL;
    node->next = list->tail;

    list->head = node;
    list->size = 0;

    return RX_OK;
}

void
rx_qlist_destroy(struct rx_qlist *list)
{
    struct rx_qlist_node *node, *next;

    node = list->head->next;

    while (node != NULL)
    {
        next = node->next;
        free(node);
        node = next;
    }

    free(list->head);

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

int
rx_qlist_add(struct rx_qlist *list, const char *value, size_t len, float weight)
{
    if (value == NULL || len == 0)
        return RX_ERROR;

    struct rx_qlist_node *newNode, *curr;

    newNode = malloc(sizeof(struct rx_qlist_node));

    if (newNode == NULL)
        return RX_ALLOC_FAILED;

    len = len > RX_QLIST_VALUE_SIZE ? RX_QLIST_VALUE_SIZE : len;

    memcpy(newNode->value, value, len);
    newNode->value[len] = '\0';
    newNode->weight     = weight;

    curr = list->head;

    for (; curr->next != NULL && curr->next->weight >= weight;
         curr = curr->next)
    {
    }

    newNode->prev = curr;
    newNode->next = curr->next;

    if (curr->next != NULL)
        curr->next->prev = newNode;
    else
        list->tail = newNode;

    curr->next = newNode;
    list->size++;

    return RX_OK;
}
