#include "unity.h"
#include "scheduler.h"
#include "process.h"
#include "priority.h"
#include <stdlib.h>

Scheduler* sched;

void setUp(void) {
    sched = create_priority_scheduler();
}

void tearDown(void) {
    scheduler_destroy(sched);
}

void test_empty_queue(void) {
    Process* p = scheduler_get_next_process(sched);
    TEST_ASSERT_NULL(p);
}

void test_higher_priority_first(void) {
    Process p1; p1.id = 1; p1.priority = 10; p1.ready_queue_arrival_time = 0;
    Process p2; p2.id = 2; p2.priority = 5;  p2.ready_queue_arrival_time = 1;
    Process p3; p3.id = 3; p3.priority = 20; p3.ready_queue_arrival_time = 2;

    scheduler_enqueue_process(sched, &p1);
    scheduler_enqueue_process(sched, &p2);
    scheduler_enqueue_process(sched, &p3);


    TEST_ASSERT_EQUAL_INT(2, scheduler_get_next_process(sched)->id);
    TEST_ASSERT_EQUAL_INT(1, scheduler_get_next_process(sched)->id);
    TEST_ASSERT_EQUAL_INT(3, scheduler_get_next_process(sched)->id);
    TEST_ASSERT_NULL(scheduler_get_next_process(sched));
}

void test_tie_breaker_arrival_time(void) {

    Process p1; p1.id = 1; p1.priority = 10; p1.ready_queue_arrival_time = 5;
    Process p2; p2.id = 2; p2.priority = 10; p2.ready_queue_arrival_time = 2;
    
    scheduler_enqueue_process(sched, &p1);
    scheduler_enqueue_process(sched, &p2);

    // Ordem esperada: p2 (chegou em t=2), depois p1 (chegou em t=5)
    TEST_ASSERT_EQUAL_INT(2, scheduler_get_next_process(sched)->id);
    TEST_ASSERT_EQUAL_INT(1, scheduler_get_next_process(sched)->id);
}

void test_tie_breaker_id(void) {

    Process p1; p1.id = 5; p1.priority = 10; p1.ready_queue_arrival_time = 2;
    Process p2; p2.id = 2; p2.priority = 10; p2.ready_queue_arrival_time = 2;
    
    scheduler_enqueue_process(sched, &p1);
    scheduler_enqueue_process(sched, &p2);

    TEST_ASSERT_EQUAL_INT(2, scheduler_get_next_process(sched)->id);
    TEST_ASSERT_EQUAL_INT(5, scheduler_get_next_process(sched)->id);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_empty_queue);
    RUN_TEST(test_higher_priority_first);
    RUN_TEST(test_tie_breaker_arrival_time);
    RUN_TEST(test_tie_breaker_id);
    return UNITY_END();
}
