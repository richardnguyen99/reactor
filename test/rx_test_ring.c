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

#include <unity/unity.h>
#include <unity/unity_fixture.h>

#include <rx_config.h>
#include <rx_core.h>

static int counter = 0;
static int *ptr;
struct rx_ring ring;

static void *
counter_handler(void *arg)
{
    NOOP(arg);

    counter++;
    return NULL;
}

static void *
counter_handler_with_arg(void *arg)
{
    int *_ptr = (int *)arg;

    (*_ptr)++;
    return _ptr;
}

TEST_GROUP(RX_RING);

TEST_SETUP(RX_RING)
{
}

TEST_TEAR_DOWN(RX_RING)
{
}

TEST(RX_RING, AddTaskToRingTest)
{
    struct rx_task *task = malloc(sizeof(struct rx_task));
    TEST_ASSERT_NOT_NULL(task);

    task->handle = NULL;
    task->arg    = NULL;
    rx_ring_push(&ring, task);

    TEST_ASSERT_EQUAL(1, ring.size);
    TEST_ASSERT_EQUAL(0, ring.out);
    TEST_ASSERT_EQUAL(1, ring.in);

    TEST_ASSERT_EQUAL_PTR(task, ring.tasks[0]);
    TEST_PASS_MESSAGE("Add task to ring test passed");
}

TEST(RX_RING, RemoveTaskFromRingTest)
{
    TEST_ASSERT_EQUAL(1, ring.size);

    struct rx_task *task = rx_ring_pop(&ring);

    TEST_ASSERT_EQUAL(0, ring.size);
    TEST_ASSERT_EQUAL(1, ring.out);
    TEST_ASSERT_EQUAL(1, ring.in);
    TEST_ASSERT_NULL(ring.tasks[0]);
    TEST_ASSERT_NOT_NULL(task);

    TEST_ASSERT_NULL(task->handle);
    TEST_ASSERT_NULL(task->arg);

    free(task);

    TEST_PASS_MESSAGE("Remove task from ring test passed");
}

TEST(RX_RING, RemoveTaskFromEmptyRingTest)
{
    TEST_ASSERT_EQUAL(0, ring.size);

    struct rx_task *task = rx_ring_pop(&ring);

    TEST_ASSERT_EQUAL(0, ring.size);
    TEST_ASSERT_EQUAL(1, ring.out);
    TEST_ASSERT_EQUAL(1, ring.in);
    TEST_ASSERT_NULL(ring.tasks[ring.out]);
    TEST_ASSERT_NULL(task);

    TEST_PASS_MESSAGE("Remove task from empty ring test passed");
}

TEST(RX_RING, AddTaskToFullRingTest)
{
    for (size_t i = 0; i < ring.cap; i++)
    {
        struct rx_task *task = malloc(sizeof(struct rx_task));
        TEST_ASSERT_NOT_NULL(task);

        task->handle = NULL;
        task->arg    = NULL;
        rx_ring_push(&ring, task);

        TEST_ASSERT_EQUAL(i + 1, ring.size);
        TEST_ASSERT_EQUAL(1, ring.out);
        TEST_ASSERT_EQUAL((i + 2) % ring.cap, ring.in);

        TEST_ASSERT_EQUAL_PTR(task, ring.tasks[(i + 1) % ring.cap]);
    }

    TEST_ASSERT_EQUAL(ring.cap, ring.size);

    struct rx_task *task = malloc(sizeof(struct rx_task));
    TEST_ASSERT_NOT_NULL(task);

    task->handle = NULL;
    task->arg    = NULL;
    rx_ring_push(&ring, task);

    TEST_ASSERT_EQUAL(ring.cap, ring.size);
    TEST_ASSERT_EQUAL(1, ring.out);
    TEST_ASSERT_EQUAL(1, ring.in);

    free(task);

    TEST_PASS_MESSAGE("Add task to full ring test passed");
}

TEST(RX_RING, SetEmptyRingTest)
{
    for (size_t i = 0; i < ring.cap; i++)
    {
        struct rx_task *task = rx_ring_pop(&ring);

        TEST_ASSERT_EQUAL(ring.cap - i - 1, ring.size);
        TEST_ASSERT_EQUAL((i + 2) % ring.cap, ring.out);

        TEST_ASSERT_NOT_NULL(task);
        TEST_ASSERT_NULL(task->handle);
        TEST_ASSERT_NULL(task->arg);

        free(task);
    }

    TEST_ASSERT_EQUAL(0, ring.size);
    TEST_ASSERT_EQUAL(1, ring.out);
    TEST_ASSERT_EQUAL(1, ring.in);

    ring.size = 0;
    ring.out  = 0;
    ring.in   = 0;

    TEST_PASS_MESSAGE("Set empty ring test passed");
}

TEST(RX_RING, AddTaskWithHandlerToRingTest)
{
    struct rx_task *task = malloc(sizeof(struct rx_task));
    TEST_ASSERT_NOT_NULL(task);

    task->handle = counter_handler;
    task->arg    = NULL;
    rx_ring_push(&ring, task);

    TEST_ASSERT_EQUAL(1, ring.size);
    TEST_ASSERT_EQUAL(0, ring.out);
    TEST_ASSERT_EQUAL(1, ring.in);

    TEST_ASSERT_EQUAL_PTR(task, ring.tasks[0]);
    TEST_PASS_MESSAGE("Add task with handler to ring test passed");
}

TEST(RX_RING, RemoveTaskWithHandlerToRingTest)
{
    struct rx_task *task = rx_ring_pop(&ring);

    TEST_ASSERT_EQUAL(0, ring.size);
    TEST_ASSERT_NOT_NULL(task);
    TEST_ASSERT_NOT_NULL(task->handle);
    TEST_ASSERT_NULL(task->arg);

    task->handle(task->arg);

    TEST_ASSERT_EQUAL(1, counter);

    free(task);

    ring.size = 0;
    ring.in   = 0;
    ring.out  = 0;

    TEST_PASS_MESSAGE("Remove task with handler from ring test passed");
}

TEST(RX_RING, AddTaskWithArgumentToRingTest)
{
    struct rx_task *task1 = malloc(sizeof(struct rx_task));
    TEST_ASSERT_NOT_NULL(task1);

    task1->handle = counter_handler_with_arg;
    task1->arg    = ptr;
    rx_ring_push(&ring, task1);

    struct rx_task *task2 = malloc(sizeof(struct rx_task));
    TEST_ASSERT_NOT_NULL(task2);

    task2->handle = counter_handler_with_arg;
    task2->arg    = ptr;
    rx_ring_push(&ring, task2);

    TEST_ASSERT_EQUAL(2, ring.size);
    TEST_ASSERT_EQUAL(0, ring.out);
    TEST_ASSERT_EQUAL(2, ring.in);

    TEST_ASSERT_EQUAL_PTR(task1, ring.tasks[0]);
    TEST_ASSERT_EQUAL_PTR(task2, ring.tasks[1]);

    TEST_PASS_MESSAGE("Add tasks with argument to ring test passed");
}

TEST(RX_RING, RemoveTaskWithArgumentFromRingTest)
{
    int *ret;
    struct rx_task *task1 = rx_ring_pop(&ring);

    TEST_ASSERT_NOT_NULL(task1);
    TEST_ASSERT_NOT_NULL(task1->handle);
    TEST_ASSERT_NOT_NULL(task1->arg);

    ret = task1->handle(task1->arg);

    TEST_ASSERT_EQUAL(1, *ptr);

    TEST_ASSERT_NOT_NULL(ret);
    TEST_ASSERT_EQUAL(1, *ret);

    TEST_ASSERT_EQUAL(1, ring.size);
    TEST_ASSERT_EQUAL(1, ring.out);
    TEST_ASSERT_EQUAL(2, ring.in);

    free(task1);

    struct rx_task *task2 = rx_ring_pop(&ring);

    TEST_ASSERT_NOT_NULL(task2);
    TEST_ASSERT_NOT_NULL(task2->handle);
    TEST_ASSERT_NOT_NULL(task2->arg);

    ret = task2->handle(task2->arg);

    TEST_ASSERT_EQUAL(2, *ptr);

    TEST_ASSERT_NOT_NULL(ret);
    TEST_ASSERT_EQUAL(2, *ret);

    TEST_ASSERT_EQUAL(0, ring.size);
    TEST_ASSERT_EQUAL(2, ring.out);
    TEST_ASSERT_EQUAL(2, ring.in);

    free(task2);

    TEST_PASS_MESSAGE("Remove tasks with argument from ring test passed");
}

TEST_GROUP_RUNNER(RX_RING)
{
    rx_ring_init(&ring);
    ptr  = malloc(sizeof(int));
    *ptr = 0;

    RUN_TEST_CASE(RX_RING, AddTaskToRingTest);
    RUN_TEST_CASE(RX_RING, RemoveTaskFromRingTest);
    RUN_TEST_CASE(RX_RING, RemoveTaskFromEmptyRingTest);
    RUN_TEST_CASE(RX_RING, AddTaskToFullRingTest);
    RUN_TEST_CASE(RX_RING, SetEmptyRingTest);
    RUN_TEST_CASE(RX_RING, AddTaskWithHandlerToRingTest);
    RUN_TEST_CASE(RX_RING, RemoveTaskWithHandlerToRingTest);
    RUN_TEST_CASE(RX_RING, AddTaskWithArgumentToRingTest);
    RUN_TEST_CASE(RX_RING, RemoveTaskWithArgumentFromRingTest);

    free(ptr);
}
