#ifndef PROGRESS_H
#define PROGRESS_H

#include <stdbool.h>
#include "data_structures.h"
#include "quiz.h"

typedef struct {
    char topic_name[MAX_TOPIC_NAME_LEN];
    int attempts_count;
    int average_percentage;
    int best_percentage;
    int latest_percentage;
    int total_correct;
    int total_questions;
    char category_label[32];
    bool has_attempts;
} TopicPerformance;

typedef struct {
    int total_quizzes;
    int overall_average_percentage;
    int highest_score_percentage;
    int overall_accuracy_percentage;

    TopicPerformance topics[TOPIC_COUNT];
    int strongest_topic_index; /* -1 if no attempts */
    int weakest_topic_index;   /* -1 if no attempts */

    char adaptive_recommended_topic[MAX_TOPIC_NAME_LEN];
    char adaptive_recommendation_reason[128];
    bool has_recommendation;
} OverallProgress;

void progress_calculate(const AttemptList *list, OverallProgress *out_progress);
bool attempts_load_from_file(AttemptList *list, const char *filepath, const char *student_id);
bool attempts_append_to_file(const char *filepath, const QuizAttempt *attempt);

#endif /* PROGRESS_H */
