#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdbool.h>
#include <stddef.h>

/* ========================================================================= */
/* 1. QUEUE: Question Order Management                                       */
/* Used to manage the sequence of questions presented during a quiz session. */
/* ========================================================================= */

typedef struct QueueNode {
    int question_index;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
    int count;
} Queue;

void queue_init(Queue *q);
bool queue_is_empty(const Queue *q);
bool queue_enqueue(Queue *q, int question_index);
bool queue_dequeue(Queue *q, int *out_question_index);
int queue_count(const Queue *q);
void queue_free(Queue *q);

/* ========================================================================= */
/* 2. STACK: Answer History & Undo Support                                   */
/* Used to record user answer changes so students can "Undo" their last      */
/* choice cleanly without corrupting the quiz state.                         */
/* ========================================================================= */

typedef struct {
    int question_index;
    int previous_option; /* -1 for unselected, 0..3 for chosen */
    int new_option;
} AnswerAction;

typedef struct StackNode {
    AnswerAction action;
    struct StackNode *next;
} StackNode;

typedef struct {
    StackNode *top;
    int count;
} Stack;

void stack_init(Stack *s);
bool stack_is_empty(const Stack *s);
bool stack_push(Stack *s, AnswerAction action);
bool stack_pop(Stack *s, AnswerAction *out_action);
bool stack_peek(const Stack *s, AnswerAction *out_action);
int stack_count(const Stack *s);
void stack_free(Stack *s);

/* ========================================================================= */
/* 3. LINKED LIST: Quiz Attempt History                                      */
/* Dynamically tracks all completed quiz sessions for progress analytics.    */
/* ========================================================================= */

#define MAX_TOPIC_NAME_LEN 48
#define MAX_DATE_LEN 24
#define MAX_QUIZ_QUESTIONS 16

typedef struct {
    char attempt_id[32];
    char student_id[32];
    char topic_name[MAX_TOPIC_NAME_LEN];
    int total_questions;
    int correct_answers;
    int incorrect_answers;
    int unanswered;
    int score_percentage;
    char timestamp[MAX_DATE_LEN];
    /* Detailed per-question answer snapshot for review reconstruction */
    int question_ids[MAX_QUIZ_QUESTIONS];
    int user_answers[MAX_QUIZ_QUESTIONS];   /* -1 if unanswered, 0..3 */
    int correct_answers_key[MAX_QUIZ_QUESTIONS];
} QuizAttempt;

typedef struct AttemptNode {
    QuizAttempt attempt;
    struct AttemptNode *next;
} AttemptNode;

typedef struct {
    AttemptNode *head;
    int count;
} AttemptList;

void attempt_list_init(AttemptList *list);
bool attempt_list_add(AttemptList *list, const QuizAttempt *attempt);
int attempt_list_count(const AttemptList *list);
const QuizAttempt* attempt_list_get_at(const AttemptList *list, int index);
const QuizAttempt* attempt_list_find_by_id(const AttemptList *list, const char *attempt_id);
void attempt_list_free(AttemptList *list);

#endif /* DATA_STRUCTURES_H */
