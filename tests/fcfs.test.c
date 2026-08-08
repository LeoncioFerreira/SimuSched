#include "unity.h"
#include "scheduler.h"
#include "process.h"
#include "fcfs.h"
#include <stdlib.h>

Scheduler* sched;

void setUp(void) {
    sched = create_fcfs_scheduler();
}

void tearDown(void) {
    free(sched->state);
    free(sched);
}

void test_empty_queue(void) {
    Process* p = sched->get_next_process(sched);
    TEST_ASSERT_NULL(p);
}

void test_fcfs_order(void) {

    Process p1; p1.id = 1;
    Process p2; p2.id = 2;
    Process p3; p3.id = 3;

    sched->enqueue_process(sched, &p1);
    sched->enqueue_process(sched, &p2);
    sched->enqueue_process(sched, &p3);

    TEST_ASSERT_EQUAL_INT(1, sched->get_next_process(sched)->id);
    TEST_ASSERT_EQUAL_INT(2, sched->get_next_process(sched)->id);
    TEST_ASSERT_EQUAL_INT(3, sched->get_next_process(sched)->id);
    TEST_ASSERT_NULL(sched->get_next_process(sched));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_empty_queue);
    RUN_TEST(test_fcfs_order);
    return UNITY_END();
}
