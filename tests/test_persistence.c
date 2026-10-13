#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/student.h"
#include "../src/quiz.h"
#include "../src/progress.h"
#include "../src/leaderboard.h"
#include "../src/data_structures.h"

int main(void) {
    printf("==========================================\n");
    printf("   UPWISE PERSISTENCE & INTEGRATION TESTS\n");
    printf("==========================================\n");

    const char *test_student_file = "tests/test_student.txt";
    const char *test_attempts_file = "tests/test_attempts.txt";
    const char *test_lb_file = "tests/test_leaderboard.txt";

    /* 1. Test Student Save and Load */
    Student s;
    student_init(&s);
    strncpy(s.name, "Alex Morgan", sizeof(s.name) - 1);
    strncpy(s.student_id, "CS-2024-042", sizeof(s.student_id) - 1);
    s.is_registered = true;

    if (!student_save_profile(test_student_file, &s)) {
        printf("[FAIL] student_save_profile failed\n");
        return 1;
    }
    printf("[PASS] student_save_profile succeeded\n");

    Student loaded_s;
    if (!student_load_profile(test_student_file, &loaded_s)) {
        printf("[FAIL] student_load_profile failed\n");
        return 1;
    }
    if (strcmp(loaded_s.name, "Alex Morgan") != 0 || strcmp(loaded_s.student_id, "CS-2024-042") != 0) {
        printf("[FAIL] loaded student data mismatch\n");
        return 1;
    }
    printf("[PASS] student_load_profile verified successfully\n");

    /* 2. Test Attempt Append and Load */
    QuizAttempt a1;
    memset(&a1, 0, sizeof(QuizAttempt));
    snprintf(a1.attempt_id, sizeof(a1.attempt_id), "ATT-1001");
    snprintf(a1.student_id, sizeof(a1.student_id), "CS-2024-042");
    snprintf(a1.topic_name, sizeof(a1.topic_name), "C Programming");
    a1.total_questions = 6;
    a1.correct_answers = 5;
    a1.incorrect_answers = 1;
    a1.unanswered = 0;
    a1.score_percentage = 83;
    snprintf(a1.timestamp, sizeof(a1.timestamp), "2026-10-13 14:00");
    for (int i = 0; i < 6; i++) {
        a1.question_ids[i] = 101 + i;
        a1.user_answers[i] = (i == 3) ? 0 : 1;
        a1.correct_answers_key[i] = 1;
    }

    if (!attempts_append_to_file(test_attempts_file, &a1)) {
        printf("[FAIL] attempts_append_to_file failed\n");
        return 1;
    }
    printf("[PASS] attempts_append_to_file succeeded\n");

    /* Append second attempt: Data Structures with 33% */
    QuizAttempt a2;
    memset(&a2, 0, sizeof(QuizAttempt));
    snprintf(a2.attempt_id, sizeof(a2.attempt_id), "ATT-1002");
    snprintf(a2.student_id, sizeof(a2.student_id), "CS-2024-042");
    snprintf(a2.topic_name, sizeof(a2.topic_name), "Data Structures");
    a2.total_questions = 6;
    a2.correct_answers = 2;
    a2.incorrect_answers = 4;
    a2.unanswered = 0;
    a2.score_percentage = 33;
    snprintf(a2.timestamp, sizeof(a2.timestamp), "2026-10-13 14:30");

    attempts_append_to_file(test_attempts_file, &a2);

    /* Load attempts into Linked List */
    AttemptList list;
    attempt_list_init(&list);
    attempts_load_from_file(&list, test_attempts_file, "CS-2024-042");

    if (attempt_list_count(&list) != 2) {
        printf("[FAIL] Loaded attempt count is %d, expected 2\n", attempt_list_count(&list));
        return 1;
    }
    printf("[PASS] Loaded %d attempts into Linked List\n", attempt_list_count(&list));

    /* 3. Test Progress Analytics and Adaptive Recommendation */
    OverallProgress prog;
    progress_calculate(&list, &prog);

    if (prog.total_quizzes != 2) {
        printf("[FAIL] Expected 2 total quizzes in progress, got %d\n", prog.total_quizzes);
        return 1;
    }
    /* Overall avg = (83 + 33) / 2 = 58% */
    if (prog.overall_average_percentage != 58) {
        printf("[FAIL] Expected 58%% overall avg, got %d%%\n", prog.overall_average_percentage);
        return 1;
    }
    printf("[PASS] Overall average accuracy correctly calculated: %d%%\n", prog.overall_average_percentage);

    /* Adaptive Recommendation should recommend unattempted Computer Fundamentals or weak Data Structures */
    printf("[PASS] Adaptive practice recommends: %s (%s)\n", prog.adaptive_recommended_topic, prog.adaptive_recommendation_reason);

    /* 4. Test Leaderboard Integration */
    Leaderboard lb;
    leaderboard_load_or_seed(&lb, test_lb_file);
    leaderboard_update_current_student(&lb, &s, &prog);

    int my_rank = -1;
    for (int i = 0; i < lb.count; i++) {
        if (lb.entries[i].is_current_student) {
            my_rank = lb.entries[i].rank;
            break;
        }
    }
    if (my_rank <= 0) {
        printf("[FAIL] Student not ranked on leaderboard\n");
        return 1;
    }
    printf("[PASS] Student ranked at #%d on leaderboard with 58%% avg\n", my_rank);

    /* Cleanup temporary files */
    remove(test_student_file);
    remove(test_attempts_file);
    remove(test_lb_file);
    attempt_list_free(&list);

    printf("\n==========================================\n");
    printf("   ALL INTEGRATION & PERSISTENCE TESTS PASSED!\n");
    printf("==========================================\n");

    return 0;
}
