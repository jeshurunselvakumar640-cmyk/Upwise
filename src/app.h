#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include "student.h"
#include "quiz.h"
#include "progress.h"
#include "leaderboard.h"
#include "ui.h"

typedef enum {
    SCREEN_PROFILE,
    SCREEN_DASHBOARD,
    SCREEN_QUIZ,
    SCREEN_RESULTS,
    SCREEN_REVIEW,
    SCREEN_PROGRESS,
    SCREEN_LEADERBOARD,
    SCREEN_SETTINGS
} Screen;

typedef struct {
    Screen current_screen;
    Screen previous_screen;

    Student current_student;
    QuestionBank question_bank;
    QuizSession quiz_session;
    QuizAttempt last_attempt;
    AttemptList attempt_history;
    OverallProgress progress;
    Leaderboard leaderboard;

    /* Text input states */
    TextInput name_input;
    TextInput id_input;
    TextInput leaderboard_search_input;
    char validation_error[128];

    /* Modal / Dialog states */
    bool show_submit_modal;
    bool show_confirm_exit;

    /* Review screen state */
    int review_question_index;

    /* UI navigation state */
    int active_nav_index;

    /* Data file paths */
    char profile_path[128];
    char questions_path[128];
    char attempts_path[128];
    char leaderboard_path[128];

    bool should_close;
} AppState;

void app_init(AppState *app);
void app_refresh_data(AppState *app);
void app_cleanup(AppState *app);

#endif /* APP_H */
