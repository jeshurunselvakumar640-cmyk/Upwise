#ifndef QUIZ_H
#define QUIZ_H

#include <stdbool.h>
#include "data_structures.h"

#define MAX_OPTIONS 4
#define MAX_OPTION_TEXT_LEN 128
#define MAX_QUESTION_TEXT_LEN 256
#define MAX_BANK_QUESTIONS 64

typedef enum {
    TOPIC_C_PROGRAMMING = 0,
    TOPIC_DATA_STRUCTURES,
    TOPIC_COMPUTER_FUNDAMENTALS,
    TOPIC_COUNT
} TopicID;

typedef struct {
    int id;
    char topic[MAX_TOPIC_NAME_LEN];
    char text[MAX_QUESTION_TEXT_LEN];
    char options[MAX_OPTIONS][MAX_OPTION_TEXT_LEN];
    int correct_index; /* 0 to 3 */
    char difficulty[16]; /* "Easy", "Medium", "Hard" */
} Question;

typedef struct {
    Question questions[MAX_BANK_QUESTIONS];
    int count;
} QuestionBank;

typedef struct {
    char topic[MAX_TOPIC_NAME_LEN];
    Question active_questions[MAX_QUIZ_QUESTIONS];
    int user_answers[MAX_QUIZ_QUESTIONS]; /* -1 if unselected, 0..3 if selected */
    int total_questions;
    int current_index;

    Queue question_queue; /* Question sequence manager */
    Stack undo_stack;     /* Action stack for Undo Last Answer */

    bool is_submitted;
    bool is_active;
    float time_remaining; /* Countdown timer in seconds */
    float total_duration;

    /* Result calculations after submission */
    int correct_count;
    int incorrect_count;
    int unanswered_count;
    int score_percentage;
    char performance_category[32];
} QuizSession;

/* Question bank management */
void question_bank_init(QuestionBank *bank);
bool question_bank_load_from_file(QuestionBank *bank, const char *filepath);
bool question_bank_save_to_file(const QuestionBank *bank, const char *filepath);
void question_bank_load_defaults(QuestionBank *bank);
int question_bank_get_by_topic(const QuestionBank *bank, const char *topic, Question *out_questions, int max_out);

/* Quiz session operations */
void quiz_session_init(QuizSession *session);
bool quiz_session_start(QuizSession *session, const QuestionBank *bank, const char *topic, float time_limit_seconds);
bool quiz_session_select_answer(QuizSession *session, int option_index);
bool quiz_session_undo_answer(QuizSession *session);
bool quiz_session_next(QuizSession *session);
bool quiz_session_prev(QuizSession *session);
bool quiz_session_jump_to(QuizSession *session, int index);
void quiz_session_submit(QuizSession *session, QuizAttempt *out_attempt, const char *student_id);
void quiz_session_cleanup(QuizSession *session);

const char* topic_id_to_name(TopicID id);
const char* topic_id_to_desc(TopicID id);

#endif /* QUIZ_H */
