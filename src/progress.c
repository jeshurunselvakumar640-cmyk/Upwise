#include "progress.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void progress_calculate(const AttemptList *list, OverallProgress *out_progress) {
    if (!out_progress) return;
    memset(out_progress, 0, sizeof(OverallProgress));

    /* Initialize topic slots */
    for (int t = 0; t < TOPIC_COUNT; t++) {
        TopicPerformance *tp = &out_progress->topics[t];
        strncpy(tp->topic_name, topic_id_to_name((TopicID)t), sizeof(tp->topic_name) - 1);
        tp->attempts_count = 0;
        tp->average_percentage = 0;
        tp->best_percentage = 0;
        tp->latest_percentage = 0;
        tp->total_correct = 0;
        tp->total_questions = 0;
        tp->has_attempts = false;
        strncpy(tp->category_label, "Not attempted", sizeof(tp->category_label) - 1);
    }

    out_progress->strongest_topic_index = -1;
    out_progress->weakest_topic_index = -1;
    out_progress->has_recommendation = false;

    if (!list || list->count == 0) {
        snprintf(out_progress->adaptive_recommended_topic, sizeof(out_progress->adaptive_recommended_topic), "C Programming");
        snprintf(out_progress->adaptive_recommendation_reason, sizeof(out_progress->adaptive_recommendation_reason),
                 "Complete your first quiz to unlock personalized performance recommendations.");
        return;
    }

    out_progress->total_quizzes = list->count;
    int sum_percentage = 0;
    int grand_total_correct = 0;
    int grand_total_questions = 0;
    int highest_score = 0;

    int topic_score_sum[TOPIC_COUNT] = {0};

    /* Traverse AttemptList (Linked List!) */
    AttemptNode *curr = list->head;
    while (curr != NULL) {
        const QuizAttempt *a = &curr->attempt;
        sum_percentage += a->score_percentage;
        grand_total_correct += a->correct_answers;
        grand_total_questions += a->total_questions;

        if (a->score_percentage > highest_score) {
            highest_score = a->score_percentage;
        }

        /* Match topic */
        for (int t = 0; t < TOPIC_COUNT; t++) {
            if (strcmp(a->topic_name, topic_id_to_name((TopicID)t)) == 0) {
                TopicPerformance *tp = &out_progress->topics[t];
                tp->has_attempts = true;
                tp->attempts_count++;
                topic_score_sum[t] += a->score_percentage;
                tp->total_correct += a->correct_answers;
                tp->total_questions += a->total_questions;
                if (a->score_percentage > tp->best_percentage) {
                    tp->best_percentage = a->score_percentage;
                }
                tp->latest_percentage = a->score_percentage;
                break;
            }
        }

        curr = curr->next;
    }

    out_progress->highest_score_percentage = highest_score;
    if (out_progress->total_quizzes > 0) {
        out_progress->overall_average_percentage = sum_percentage / out_progress->total_quizzes;
    }
    if (grand_total_questions > 0) {
        out_progress->overall_accuracy_percentage = (grand_total_correct * 100) / grand_total_questions;
    }

    int best_top_avg = -1;
    int worst_top_avg = 101;

    for (int t = 0; t < TOPIC_COUNT; t++) {
        TopicPerformance *tp = &out_progress->topics[t];
        if (tp->has_attempts && tp->attempts_count > 0) {
            tp->average_percentage = topic_score_sum[t] / tp->attempts_count;

            if (tp->average_percentage < 50) {
                strncpy(tp->category_label, "Needs Improvement", sizeof(tp->category_label) - 1);
            } else if (tp->average_percentage < 80) {
                strncpy(tp->category_label, "Developing", sizeof(tp->category_label) - 1);
            } else {
                strncpy(tp->category_label, "Proficient", sizeof(tp->category_label) - 1);
            }

            if (tp->average_percentage > best_top_avg) {
                best_top_avg = tp->average_percentage;
                out_progress->strongest_topic_index = t;
            }
            if (tp->average_percentage < worst_top_avg) {
                worst_top_avg = tp->average_percentage;
                out_progress->weakest_topic_index = t;
            }
        }
    }

    /* ===================================================================== */
    /* ADAPTIVE RECOMMENDATION ALGORITHM (Requirement 10)                    */
    /* ===================================================================== */
    /* 1. If any topic has not been attempted, encourage student to try it   */
    /* 2. Prioritize topic with < 50% average (lowest score)                 */
    /* 3. If none < 50%, recommend lowest developing topic (50-79%)          */
    /* 4. If all proficient (>= 80%), recommend topic for master revision    */
    /* ===================================================================== */

    int unattempted_topic = -1;
    for (int t = 0; t < TOPIC_COUNT; t++) {
        if (!out_progress->topics[t].has_attempts) {
            unattempted_topic = t;
            break;
        }
    }

    if (unattempted_topic != -1) {
        strncpy(out_progress->adaptive_recommended_topic, topic_id_to_name((TopicID)unattempted_topic), sizeof(out_progress->adaptive_recommended_topic) - 1);
        snprintf(out_progress->adaptive_recommendation_reason, sizeof(out_progress->adaptive_recommendation_reason),
                 "Explore this topic to complete your diagnostic baseline.");
        out_progress->has_recommendation = true;
    } else if (worst_top_avg < 50 && out_progress->weakest_topic_index >= 0) {
        int w = out_progress->weakest_topic_index;
        strncpy(out_progress->adaptive_recommended_topic, topic_id_to_name((TopicID)w), sizeof(out_progress->adaptive_recommended_topic) - 1);
        snprintf(out_progress->adaptive_recommendation_reason, sizeof(out_progress->adaptive_recommendation_reason),
                 "Current average is %d%%. Prioritize practice here to strengthen core foundations.",
                 out_progress->topics[w].average_percentage);
        out_progress->has_recommendation = true;
    } else if (worst_top_avg < 80 && out_progress->weakest_topic_index >= 0) {
        int w = out_progress->weakest_topic_index;
        strncpy(out_progress->adaptive_recommended_topic, topic_id_to_name((TopicID)w), sizeof(out_progress->adaptive_recommended_topic) - 1);
        snprintf(out_progress->adaptive_recommendation_reason, sizeof(out_progress->adaptive_recommendation_reason),
                 "Current average is %d%%. Continued practice will lift you into the Proficient band.",
                 out_progress->topics[w].average_percentage);
        out_progress->has_recommendation = true;
    } else if (out_progress->weakest_topic_index >= 0) {
        int w = out_progress->weakest_topic_index;
        strncpy(out_progress->adaptive_recommended_topic, topic_id_to_name((TopicID)w), sizeof(out_progress->adaptive_recommended_topic) - 1);
        snprintf(out_progress->adaptive_recommendation_reason, sizeof(out_progress->adaptive_recommendation_reason),
                 "Proficient across all topics! Revise %s to target a perfect 100%%.",
                 topic_id_to_name((TopicID)w));
        out_progress->has_recommendation = true;
    }
}

bool attempts_load_from_file(AttemptList *list, const char *filepath, const char *student_id) {
    if (!list || !filepath) return false;
    attempt_list_free(list);
    attempt_list_init(list);

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        /* File doesn't exist yet, which is normal on first start */
        return true;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        QuizAttempt a;
        memset(&a, 0, sizeof(QuizAttempt));

        /* Format:
           ID|STUDENT_ID|TOPIC|TOTAL|CORRECT|INCORRECT|UNANSWERED|PERCENTAGE|TIMESTAMP|Q_IDS|USER_ANSWERS|CORRECT_KEY */
        char *token = strtok(line, "|");
        if (!token) continue;
        strncpy(a.attempt_id, token, sizeof(a.attempt_id) - 1);

        token = strtok(NULL, "|");
        if (!token) continue;
        strncpy(a.student_id, token, sizeof(a.student_id) - 1);

        /* Filter by student_id if provided */
        if (student_id && strlen(student_id) > 0 && strcmp(a.student_id, student_id) != 0) {
            continue;
        }

        token = strtok(NULL, "|");
        if (!token) continue;
        strncpy(a.topic_name, token, sizeof(a.topic_name) - 1);

        token = strtok(NULL, "|");
        if (!token) continue;
        a.total_questions = atoi(token);

        token = strtok(NULL, "|");
        if (!token) continue;
        a.correct_answers = atoi(token);

        token = strtok(NULL, "|");
        if (!token) continue;
        a.incorrect_answers = atoi(token);

        token = strtok(NULL, "|");
        if (!token) continue;
        a.unanswered = atoi(token);

        token = strtok(NULL, "|");
        if (!token) continue;
        a.score_percentage = atoi(token);

        token = strtok(NULL, "|");
        if (!token) continue;
        strncpy(a.timestamp, token, sizeof(a.timestamp) - 1);

        /* Optional answer snapshot fields */
        token = strtok(NULL, "|");
        if (token) {
            char *sub = strtok(token, ",");
            int idx = 0;
            while (sub && idx < MAX_QUIZ_QUESTIONS) {
                a.question_ids[idx++] = atoi(sub);
                sub = strtok(NULL, ",");
            }
        }

        token = strtok(NULL, "|");
        if (token) {
            char *sub = strtok(token, ",");
            int idx = 0;
            while (sub && idx < MAX_QUIZ_QUESTIONS) {
                a.user_answers[idx++] = atoi(sub);
                sub = strtok(NULL, ",");
            }
        }

        token = strtok(NULL, "|");
        if (token) {
            char *sub = strtok(token, ",");
            int idx = 0;
            while (sub && idx < MAX_QUIZ_QUESTIONS) {
                a.correct_answers_key[idx++] = atoi(sub);
                sub = strtok(NULL, ",");
            }
        }

        attempt_list_add(list, &a);
    }

    fclose(fp);
    return true;
}

bool attempts_append_to_file(const char *filepath, const QuizAttempt *attempt) {
    if (!filepath || !attempt) return false;

    FILE *fp = fopen(filepath, "a");
    if (!fp) return false;

    /* Write main fields */
    fprintf(fp, "%s|%s|%s|%d|%d|%d|%d|%d|%s|",
            attempt->attempt_id,
            attempt->student_id,
            attempt->topic_name,
            attempt->total_questions,
            attempt->correct_answers,
            attempt->incorrect_answers,
            attempt->unanswered,
            attempt->score_percentage,
            attempt->timestamp);

    /* Write question IDs */
    for (int i = 0; i < attempt->total_questions; i++) {
        fprintf(fp, "%d%s", attempt->question_ids[i], (i + 1 < attempt->total_questions) ? "," : "");
    }
    fprintf(fp, "|");

    /* Write user answers */
    for (int i = 0; i < attempt->total_questions; i++) {
        fprintf(fp, "%d%s", attempt->user_answers[i], (i + 1 < attempt->total_questions) ? "," : "");
    }
    fprintf(fp, "|");

    /* Write correct answers key */
    for (int i = 0; i < attempt->total_questions; i++) {
        fprintf(fp, "%d%s", attempt->correct_answers_key[i], (i + 1 < attempt->total_questions) ? "," : "");
    }
    fprintf(fp, "\n");

    fclose(fp);
    return true;
}
