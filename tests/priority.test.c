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
    // Como os testes podem deixar a fila suja (não vazia),
    // o ideal seria esvaziar tudo para evitar memory leaks, 
    // mas para simplificar no test-case:
    free(sched->state);
    free(sched);
}

void test_empty_queue(void) {
    Process* p = sched->get_next_process(sched);
    TEST_ASSERT_NULL(p);
}

void test_higher_priority_first(void) {
    Process p1; p1.id = 1; p1.priority = 10; p1.ready_queue_arrival_time = 0; p1.cpu_burst = 5;
    Process p2; p2.id = 2; p2.priority = 50; p2.ready_queue_arrival_time = 1; p2.cpu_burst = 5;
    Process p3; p3.id = 3; p3.priority = 20; p3.ready_queue_arrival_time = 2; p3.cpu_burst = 5;

    sched->enqueue_process(sched, &p1);
    sched->enqueue_process(sched, &p2);
    sched->enqueue_process(sched, &p3);

    // Ordem esperada: p2 (prioridade 50), depois p3 (prioridade 20), depois p1 (prioridade 10)
    TEST_ASSERT_EQUAL_INT(2, sched->get_next_process(sched)->id);
    TEST_ASSERT_EQUAL_INT(3, sched->get_next_process(sched)->id);
    TEST_ASSERT_EQUAL_INT(1, sched->get_next_process(sched)->id);
    TEST_ASSERT_NULL(sched->get_next_process(sched));
}

void test_tie_breaker_arrival_time(void) {
    // Mesma prioridade, mas p2 chegou antes
    Process p1; p1.id = 1; p1.priority = 10; p1.ready_queue_arrival_time = 5; p1.cpu_burst = 5;
    Process p2; p2.id = 2; p2.priority = 10; p2.ready_queue_arrival_time = 2; p2.cpu_burst = 5;
    
    sched->enqueue_process(sched, &p1);
    sched->enqueue_process(sched, &p2);

    // Ordem esperada: p2 (chegou em t=2), depois p1 (chegou em t=5)
    TEST_ASSERT_EQUAL_INT(2, sched->get_next_process(sched)->id);
    TEST_ASSERT_EQUAL_INT(1, sched->get_next_process(sched)->id);
}

void test_tie_breaker_id(void) {
    // Mesma prioridade e mesmo tempo de chegada, p2 tem ID menor
    Process p1; p1.id = 5; p1.priority = 10; p1.ready_queue_arrival_time = 2; p1.cpu_burst = 5;
    Process p2; p2.id = 2; p2.priority = 10; p2.ready_queue_arrival_time = 2; p2.cpu_burst = 5;
    
    sched->enqueue_process(sched, &p1);
    sched->enqueue_process(sched, &p2);

    // Ordem esperada: p2 (ID 2), depois p1 (ID 5)
    TEST_ASSERT_EQUAL_INT(2, sched->get_next_process(sched)->id);
    TEST_ASSERT_EQUAL_INT(5, sched->get_next_process(sched)->id);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_empty_queue);
    RUN_TEST(test_higher_priority_first);
    RUN_TEST(test_tie_breaker_arrival_time);
    RUN_TEST(test_tie_breaker_id);
    return UNITY_END();
}
