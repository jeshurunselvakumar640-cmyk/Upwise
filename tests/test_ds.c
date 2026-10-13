#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../src/data_structures.h"
#include "../src/student.h"
#include "../src/quiz.h"
#include "../src/progress.h"
#include "../src/leaderboard.h"

static int g_tests_passed = 0;
static int g_tests_total = 0;

#define TEST_ASSERT(cond, name) do { \
    g_tests_total++; \
    if (cond) { \
        printf("  [PASS] %s\n", name); \
        g_tests_passed++; \
    } else { \
        printf("  [FAIL] %s (Line %d)\n", name, __LINE__); \
    } \
} while(0)

void test_queue(void) {
    printf("\n--- Testing Queue Data Structure ---\n");
    Queue q;
    queue_init(&q);

    TEST_ASSERT(queue_is_empty(&q), "New queue is empty");
    TEST_ASSERT(queue_count(&q) == 0, "Queue count is 0");

    queue_enqueue(&q, 10);
    queue_enqueue(&q, 20);
    queue_enqueue(&q, 30);

    TEST_ASSERT(!queue_is_empty(&q), "Queue is not empty after enqueue");
    TEST_ASSERT(queue_count(&q) == 3, "Queue count is 3");

    int val = -1;
    queue_dequeue(&q, &val);
    TEST_ASSERT(val == 10, "First dequeued element is 10 (FIFO order)");

    queue_dequeue(&q, &val);
    TEST_ASSERT(val == 20, "Second dequeued element is 20");

    queue_dequeue(&q, &val);
    TEST_ASSERT(val == 30, "Third dequeued element is 30");

    TEST_ASSERT(queue_is_empty(&q), "Queue is empty after dequeuing all elements");

    queue_free(&q);
    TEST_ASSERT(queue_is_empty(&q), "Queue free resets queue cleanly");
}

void test_stack(void) {
    printf("\n--- Testing Stack Data Structure (Undo Support) ---\n");
    Stack s;
    stack_init(&s);

    TEST_ASSERT(stack_is_empty(&s), "New stack is empty");
    TEST_ASSERT(stack_count(&s) == 0, "Stack count is 0");

    AnswerAction a1 = { 0, -1, 2 };
    AnswerAction a2 = { 1, -1, 0 };
    AnswerAction a3 = { 1, 0, 3 };

    stack_push(&s, a1);
    stack_push(&s, a2);
    stack_push(&s, a3);

    TEST_ASSERT(stack_count(&s) == 3, "Stack count is 3 after 3 pushes");

    AnswerAction popped;
    stack_pop(&s, &popped);
    TEST_ASSERT(popped.question_index == 1 && popped.previous_option == 0 && popped.new_option == 3,
                "Popped element is most recent action (LIFO order)");

    stack_pop(&s, &popped);
    TEST_ASSERT(popped.question_index == 1 && popped.new_option == 0, "Second pop matches second push");

    stack_free(&s);
    TEST_ASSERT(stack_is_empty(&s), "Stack free cleans all nodes");
}

void test_linked_list(void) {
    printf("\n--- Testing Quiz Attempt Linked List ---\n");
    AttemptList list;
    attempt_list_init(&list);

    TEST_ASSERT(attempt_list_count(&list) == 0, "Attempt list count is 0");

    QuizAttempt a1;
    memset(&a1, 0, sizeof(QuizAttempt));
    snprintf(a1.attempt_id, sizeof(a1.attempt_id), "ATT-001");
    snprintf(a1.student_id, sizeof(a1.student_id), "STU-01");
    snprintf(a1.topic_name, sizeof(a1.topic_name), "C Programming");
    a1.total_questions = 6;
    a1.correct_answers = 5;
    a1.score_percentage = 83;

    QuizAttempt a2;
    memset(&a2, 0, sizeof(QuizAttempt));
    snprintf(a2.attempt_id, sizeof(a2.attempt_id), "ATT-002");
    snprintf(a2.student_id, sizeof(a2.student_id), "STU-01");
    snprintf(a2.topic_name, sizeof(a2.topic_name), "Data Structures");
    a2.total_questions = 6;
    a2.correct_answers = 3;
    a2.score_percentage = 50;

    attempt_list_add(&list, &a1);
    attempt_list_add(&list, &a2);

    TEST_ASSERT(attempt_list_count(&list) == 2, "Attempt list has 2 items");

    const QuizAttempt *retrieved = attempt_list_get_at(&list, 0);
    TEST_ASSERT(retrieved != NULL && strcmp(retrieved->attempt_id, "ATT-001") == 0, "Retrieve at index 0 works");

    const QuizAttempt *found = attempt_list_find_by_id(&list, "ATT-002");
    TEST_ASSERT(found != NULL && found->score_percentage == 50, "Find by attempt ID works");

    attempt_list_free(&list);
    TEST_ASSERT(attempt_list_count(&list) == 0, "Freeing attempt list resets count to 0");
}

void test_quicksort_and_search(void) {
    printf("\n--- Testing Quicksort & Searching Algorithms ---\n");
    Leaderboard lb;
    leaderboard_init(&lb);

    LeaderboardEntry e1 = { 0, "Alice", "STU-100", 70, 5, false };
    LeaderboardEntry e2 = { 0, "Bob",   "STU-200", 95, 8, false };
    LeaderboardEntry e3 = { 0, "Carol", "STU-300", 85, 6, false };
    LeaderboardEntry e4 = { 0, "Dave",  "STU-400", 85, 10, false }; /* Tie with Carol on avg, but Dave has more quizzes */

    lb.entries[lb.count++] = e1;
    lb.entries[lb.count++] = e2;
    lb.entries[lb.count++] = e3;
    lb.entries[lb.count++] = e4;

    leaderboard_sort(&lb);

    TEST_ASSERT(lb.entries[0].rank == 1 && strcmp(lb.entries[0].name, "Bob") == 0, "Rank 1 is Bob (95%)");
    TEST_ASSERT(lb.entries[1].rank == 2 && strcmp(lb.entries[1].name, "Dave") == 0, "Rank 2 is Dave (85%, 10 quizzes wins tie-breaker)");
    TEST_ASSERT(lb.entries[2].rank == 3 && strcmp(lb.entries[2].name, "Carol") == 0, "Rank 3 is Carol (85%, 6 quizzes)");
    TEST_ASSERT(lb.entries[3].rank == 4 && strcmp(lb.entries[3].name, "Alice") == 0, "Rank 4 is Alice (70%)");

    /* Search by ID */
    int idx_dave = leaderboard_search_by_id(&lb, "STU-400");
    TEST_ASSERT(idx_dave == 1, "Leaderboard search by ID correctly locates Dave at index 1");

    int idx_missing = leaderboard_search_by_id(&lb, "STU-999");
    TEST_ASSERT(idx_missing == -1, "Search returns -1 for nonexistent ID");
}

void test_student_validation(void) {
    printf("\n--- Testing Student Input Validation ---\n");
    char err[128];

    TEST_ASSERT(!student_validate("", "STU-01", err, sizeof(err)), "Rejects empty name");
    TEST_ASSERT(!student_validate("   ", "STU-01", err, sizeof(err)), "Rejects whitespace-only name");
    TEST_ASSERT(!student_validate("Alex", "", err, sizeof(err)), "Rejects empty student ID");
    TEST_ASSERT(student_validate("Alex Morgan", "CS-2024-001", err, sizeof(err)), "Accepts valid student credentials");
}

int main(void) {
    printf("==========================================\n");
    printf("   UPWISE DATA STRUCTURES & ALGORITHM TESTS\n");
    printf("==========================================\n");

    test_queue();
    test_stack();
    test_linked_list();
    test_quicksort_and_search();
    test_student_validation();

    printf("\n==========================================\n");
    printf("Summary: %d / %d Tests Passed.\n", g_tests_passed, g_tests_total);
    printf("==========================================\n");

    return (g_tests_passed == g_tests_total) ? 0 : 1;
}
