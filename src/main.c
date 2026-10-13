#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "compat.h"
#include "app.h"
#include "ui.h"
#include "quiz.h"
#include "student.h"
#include "progress.h"
#include "leaderboard.h"
#include "data_structures.h"

#define WINDOW_WIDTH  1200
#define WINDOW_HEIGHT 760
#define SIDEBAR_WIDTH 230

static void draw_sidebar(AppState *app);
static void screen_profile_update_draw(AppState *app);
static void screen_dashboard_update_draw(AppState *app);
static void screen_quiz_update_draw(AppState *app);
static void screen_results_update_draw(AppState *app);
static void screen_review_update_draw(AppState *app);
static void screen_progress_update_draw(AppState *app);
static void screen_leaderboard_update_draw(AppState *app);
static void screen_settings_update_draw(AppState *app);

void app_init(AppState *app) {
    memset(app, 0, sizeof(AppState));

    snprintf(app->profile_path, sizeof(app->profile_path), "data/students.txt");
    snprintf(app->questions_path, sizeof(app->questions_path), "data/questions.txt");
    snprintf(app->attempts_path, sizeof(app->attempts_path), "data/attempts.txt");
    snprintf(app->leaderboard_path, sizeof(app->leaderboard_path), "data/leaderboard.txt");

    /* Initialize Question Bank */
    question_bank_init(&app->question_bank);
    question_bank_load_from_file(&app->question_bank, app->questions_path);

    /* Initialize inputs */
    app->name_input.max_len = 48;
    app->id_input.max_len = 24;
    app->leaderboard_search_input.max_len = 24;

    /* Initialize Attempt History Linked List */
    attempt_list_init(&app->attempt_history);

    /* Attempt to load existing student profile */
    bool loaded = student_load_profile(app->profile_path, &app->current_student);
    if (loaded && app->current_student.is_registered) {
        strncpy(app->name_input.text, app->current_student.name, sizeof(app->name_input.text) - 1);
        strncpy(app->id_input.text, app->current_student.student_id, sizeof(app->id_input.text) - 1);
        app_refresh_data(app);
        app->current_screen = SCREEN_DASHBOARD;
        app->active_nav_index = 0;
    } else {
        app->current_screen = SCREEN_PROFILE;
    }
}

void app_refresh_data(AppState *app) {
    if (!app || !app->current_student.is_registered) return;

    /* Load attempts for current student into Linked List */
    attempts_load_from_file(&app->attempt_history, app->attempts_path, app->current_student.student_id);

    /* Recalculate overall progress statistics & adaptive recommendations */
    progress_calculate(&app->attempt_history, &app->progress);

    /* Reload or seed Leaderboard and update current student entry */
    leaderboard_load_or_seed(&app->leaderboard, app->leaderboard_path);
    leaderboard_update_current_student(&app->leaderboard, &app->current_student, &app->progress);
    leaderboard_save_to_file(&app->leaderboard, app->leaderboard_path);
}

void app_cleanup(AppState *app) {
    if (!app) return;
    attempt_list_free(&app->attempt_history);
    quiz_session_cleanup(&app->quiz_session);
}

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif

static AppState g_app;

#if defined(__EMSCRIPTEN__)
EMSCRIPTEN_KEEPALIVE
void set_google_user(const char *name, const char *student_id) {
    if (!name || !student_id) return;
    strncpy(g_app.current_student.name, name, sizeof(g_app.current_student.name) - 1);
    g_app.current_student.name[sizeof(g_app.current_student.name) - 1] = '\0';
    strncpy(g_app.current_student.student_id, student_id, sizeof(g_app.current_student.student_id) - 1);
    g_app.current_student.student_id[sizeof(g_app.current_student.student_id) - 1] = '\0';
    g_app.current_student.is_registered = true;

    /* Populate UI inputs */
    strncpy(g_app.name_input.text, g_app.current_student.name, sizeof(g_app.name_input.text) - 1);
    strncpy(g_app.id_input.text, g_app.current_student.student_id, sizeof(g_app.id_input.text) - 1);

    student_save_profile(g_app.profile_path, &g_app.current_student);
    app_refresh_data(&g_app);

    if (g_app.current_screen == SCREEN_PROFILE) {
        g_app.current_screen = SCREEN_DASHBOARD;
        g_app.active_nav_index = 0;
    }
}
#endif

static void update_draw_frame(void) {
    /* Update Quiz countdown timer */
    if (g_app.current_screen == SCREEN_QUIZ && g_app.quiz_session.is_active && !g_app.quiz_session.is_submitted) {
        float dt = GetFrameTime();
        g_app.quiz_session.time_remaining -= dt;
        if (g_app.quiz_session.time_remaining <= 0.0f) {
            g_app.quiz_session.time_remaining = 0.0f;
            /* Auto-submit on time expiry */
            quiz_session_submit(&g_app.quiz_session, &g_app.last_attempt, g_app.current_student.student_id);
            attempts_append_to_file(g_app.attempts_path, &g_app.last_attempt);
            app_refresh_data(&g_app);
            g_app.current_screen = SCREEN_RESULTS;
        }
    }

    BeginDrawing();
    ClearBackground(COLOR_BG_MAIN);

    /* Render Current Screen */
    switch (g_app.current_screen) {
        case SCREEN_PROFILE:
            screen_profile_update_draw(&g_app);
            break;
        case SCREEN_DASHBOARD:
            draw_sidebar(&g_app);
            screen_dashboard_update_draw(&g_app);
            break;
        case SCREEN_QUIZ:
            screen_quiz_update_draw(&g_app);
            break;
        case SCREEN_RESULTS:
            screen_results_update_draw(&g_app);
            break;
        case SCREEN_REVIEW:
            screen_review_update_draw(&g_app);
            break;
        case SCREEN_PROGRESS:
            draw_sidebar(&g_app);
            screen_progress_update_draw(&g_app);
            break;
        case SCREEN_LEADERBOARD:
            draw_sidebar(&g_app);
            screen_leaderboard_update_draw(&g_app);
            break;
        case SCREEN_SETTINGS:
            draw_sidebar(&g_app);
            screen_settings_update_draw(&g_app);
            break;
    }

    EndDrawing();
}

int main(void) {
    init_compat_layer();

#if defined(__EMSCRIPTEN__)
    SetConfigFlags(FLAG_VSYNC_HINT);
#else
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
#endif
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Upwise | Learn. Practise. Progress.");
    SetTargetFPS(60);

    ui_init();

    app_init(&g_app);

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop(update_draw_frame, 0, 1);
#else
    while (!WindowShouldClose() && !g_app.should_close) {
        update_draw_frame();
    }

    app_cleanup(&g_app);
    ui_close_fonts();
    CloseWindow();
#endif
    return 0;
}

/* ========================================================================= */
/* SIDEBAR NAVIGATION COMPONENT                                              */
/* ========================================================================= */

static void draw_sidebar(AppState *app) {
    Rectangle sidebar_rec = { 0, 0, SIDEBAR_WIDTH, WINDOW_HEIGHT };
    DrawRectangleRec(sidebar_rec, COLOR_BG_SECONDARY);
    DrawLine((int)SIDEBAR_WIDTH, 0, (int)SIDEBAR_WIDTH, WINDOW_HEIGHT, COLOR_BORDER);

    /* Brand Logo */
    ui_draw_logo((Vector2){ 24, 28 }, 0.95f, true, true);
    DrawLine(20, 80, SIDEBAR_WIDTH - 20, 80, COLOR_BORDER);

    /* Nav Items */
    const char *nav_labels[] = {
        "Dashboard",
        "Take a Quiz",
        "My Progress",
        "Leaderboard",
        "Profile & Settings",
        "Exit"
    };

    int nav_screens[] = {
        SCREEN_DASHBOARD,
        -1, /* Takes to topic selection on dashboard */
        SCREEN_PROGRESS,
        SCREEN_LEADERBOARD,
        SCREEN_SETTINGS,
        -2  /* Exit */
    };

    float start_y = 100.0f;
    float item_h = 44.0f;
    float gap = 8.0f;

    for (int i = 0; i < 6; i++) {
        Rectangle item_rec = { 14, start_y + i * (item_h + gap), SIDEBAR_WIDTH - 28, item_h };
        bool is_active = (app->active_nav_index == i);

        Vector2 mouse_pos = GetMousePosition();
        bool hovered = CheckCollisionPointRec(mouse_pos, item_rec);

        if (is_active) {
            DrawRectangleRounded(item_rec, 0.25f, 4, (Color){ 91, 140, 255, 38 });
            DrawRectangleRoundedLines(item_rec, 0.25f, 4, 1.2f, COLOR_PRIMARY);
            /* Left Active Accent Pill */
            DrawRectangleRounded((Rectangle){ item_rec.x + 3, item_rec.y + 10, 3, item_rec.height - 20 }, 0.5f, 2, COLOR_PRIMARY);
        } else if (hovered) {
            DrawRectangleRounded(item_rec, 0.25f, 4, COLOR_CARD_HOVER);
        }

        Color text_col = is_active ? COLOR_PRIMARY : (hovered ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED);
        DrawText(nav_labels[i], (int)(item_rec.x + 22), (int)(item_rec.y + 14), 14, text_col);

        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (i == 5) {
                app->should_close = true;
            } else if (i == 1) {
                app->current_screen = SCREEN_DASHBOARD;
                app->active_nav_index = 0;
            } else {
                app->active_nav_index = i;
                app->current_screen = (Screen)nav_screens[i];
            }
        }
    }

    /* Student Profile Badge at Bottom of Sidebar */
    Rectangle profile_badge = { 14, WINDOW_HEIGHT - 74, SIDEBAR_WIDTH - 28, 56 };
    ui_card(profile_badge, COLOR_CARD_BG, COLOR_BORDER, 0.2f);
    DrawCircle((int)(profile_badge.x + 24), (int)(profile_badge.y + 28), 16, COLOR_PRIMARY);
    
    char initial[2] = { 'U', '\0' };
    if (strlen(app->current_student.name) > 0) {
        initial[0] = app->current_student.name[0];
    }
    int init_w = MeasureText(initial, 16);
    DrawText(initial, (int)(profile_badge.x + 24 - init_w / 2), (int)(profile_badge.y + 20), 16, (Color){ 10, 15, 30, 255 });

    /* Student Name & ID */
    char display_name[20];
    strncpy(display_name, app->current_student.name, 15);
    display_name[15] = '\0';
    if (strlen(app->current_student.name) > 15) strcat(display_name, "..");

    DrawText(display_name, (int)(profile_badge.x + 48), (int)(profile_badge.y + 12), 13, COLOR_TEXT_PRIMARY);
    DrawText(app->current_student.student_id, (int)(profile_badge.x + 48), (int)(profile_badge.y + 30), 11, COLOR_TEXT_DARK);
}

/* ========================================================================= */
/* SCREEN 1: STUDENT PROFILE ONBOARDING                                      */
/* ========================================================================= */

static void screen_profile_update_draw(AppState *app) {
    /* Background Ambience */
    DrawCircle(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 140, 260, (Color){ 91, 140, 255, 12 });
    DrawCircle(WINDOW_WIDTH / 2 - 180, WINDOW_HEIGHT / 2 + 100, 200, (Color){ 139, 92, 246, 10 });

    /* Center Form Card */
    float card_w = 480.0f;
    float card_h = 490.0f;
    Rectangle card_rec = { (WINDOW_WIDTH - card_w) / 2.0f, (WINDOW_HEIGHT - card_h) / 2.0f, card_w, card_h };
    ui_card(card_rec, COLOR_CARD_BG, COLOR_BORDER, 0.12f);

    /* Brand Logo at Header */
    ui_draw_logo((Vector2){ card_rec.x + (card_w - 180) / 2.0f, card_rec.y + 32 }, 1.0f, true, true);

    DrawText("Student Onboarding", (int)(card_rec.x + 40), (int)(card_rec.y + 115), 20, COLOR_TEXT_PRIMARY);
    DrawText("Enter your credentials to track your academic progress.",
             (int)(card_rec.x + 40), (int)(card_rec.y + 142), 12, COLOR_TEXT_MUTED);

    /* Input: Full Name */
    DrawText("Full Name", (int)(card_rec.x + 40), (int)(card_rec.y + 175), 13, COLOR_TEXT_PRIMARY);
    Rectangle name_rec = { card_rec.x + 40, card_rec.y + 196, card_w - 80, 44 };
    ui_text_input(name_rec, &app->name_input, "e.g. Alex Morgan");

    /* Input: Student ID */
    DrawText("Student ID", (int)(card_rec.x + 40), (int)(card_rec.y + 255), 13, COLOR_TEXT_PRIMARY);
    Rectangle id_rec = { card_rec.x + 40, card_rec.y + 276, card_w - 80, 44 };
    ui_text_input(id_rec, &app->id_input, "e.g. CS-2024-042");

    /* Validation Error Display */
    if (strlen(app->validation_error) > 0) {
        DrawText(app->validation_error, (int)(card_rec.x + 40), (int)(card_rec.y + 332), 12, COLOR_ERROR);
    }

    /* Start Learning Button */
    Rectangle btn_rec = { card_rec.x + 40, card_rec.y + 360, card_w - 80, 48 };
    if (ui_button(btn_rec, "Start Learning ->", BTN_PRIMARY, true)) {
        char err[128] = {0};
        if (student_validate(app->name_input.text, app->id_input.text, err, sizeof(err))) {
            strncpy(app->current_student.name, app->name_input.text, sizeof(app->current_student.name) - 1);
            strncpy(app->current_student.student_id, app->id_input.text, sizeof(app->current_student.student_id) - 1);
            app->current_student.is_registered = true;

            student_save_profile(app->profile_path, &app->current_student);
            app_refresh_data(app);
            app->current_screen = SCREEN_DASHBOARD;
            app->active_nav_index = 0;
            app->validation_error[0] = '\0';
        } else {
            strncpy(app->validation_error, err, sizeof(app->validation_error) - 1);
        }
    }

    /* Academic Footer Note */
    DrawText("Local Offline Storage - MinGW & Raylib Engine", (int)(card_rec.x + 85), (int)(card_rec.y + 440), 11, COLOR_TEXT_DARK);
}

/* ========================================================================= */
/* SCREEN 2: DASHBOARD                                                       */
/* ========================================================================= */

static void screen_dashboard_update_draw(AppState *app) {
    float content_x = SIDEBAR_WIDTH + 32.0f;
    float content_w = WINDOW_WIDTH - SIDEBAR_WIDTH - 64.0f;

    /* Welcome Section Header */
    char greeting[128];
    snprintf(greeting, sizeof(greeting), "Welcome back, %s!", app->current_student.name);
    DrawText(greeting, (int)content_x, 30, 24, COLOR_TEXT_PRIMARY);
    DrawText("Small steps today. Greater progress tomorrow.", (int)content_x, 62, 13, COLOR_TEXT_MUTED);

    /* Statistics Cards Grid (Row of 4 Cards) */
    float card_w = (content_w - 36.0f) / 4.0f;
    float stat_y = 92.0f;
    float stat_h = 88.0f;

    char str_quizzes[16];
    snprintf(str_quizzes, sizeof(str_quizzes), "%d", app->progress.total_quizzes);
    ui_draw_stat_card((Rectangle){ content_x, stat_y, card_w, stat_h }, "Quizzes Completed", str_quizzes, "All time attempts", COLOR_PRIMARY);

    char str_avg[16];
    snprintf(str_avg, sizeof(str_avg), "%d%%", app->progress.overall_average_percentage);
    ui_draw_stat_card((Rectangle){ content_x + card_w + 12.0f, stat_y, card_w, stat_h }, "Average Score", str_avg, "Mean accuracy", COLOR_ACCENT);

    char str_best[16];
    snprintf(str_best, sizeof(str_best), "%d%%", app->progress.highest_score_percentage);
    ui_draw_stat_card((Rectangle){ content_x + (card_w + 12.0f) * 2, stat_y, card_w, stat_h }, "Best Score", str_best, "Top recorded", COLOR_SUCCESS);

    char str_acc[16];
    snprintf(str_acc, sizeof(str_acc), "%d%%", app->progress.overall_accuracy_percentage);
    ui_draw_stat_card((Rectangle){ content_x + (card_w + 12.0f) * 3, stat_y, card_w, stat_h }, "Overall Accuracy", str_acc, "Questions answered", COLOR_WARNING);

    /* Adaptive Practice Recommendation Banner (Requirement 10) */
    float banner_y = 196.0f;
    float banner_h = 76.0f;
    Rectangle banner_rec = { content_x, banner_y, content_w, banner_h };
    ui_card(banner_rec, (Color){ 20, 28, 50, 255 }, COLOR_PRIMARY, 0.16f);

    /* Left glowing icon indicator */
    DrawCircle((int)(banner_rec.x + 36), (int)(banner_rec.y + banner_h / 2), 18, (Color){ 91, 140, 255, 35 });
    DrawText("AI", (int)(banner_rec.x + 30), (int)(banner_rec.y + banner_h / 2 - 8), 14, COLOR_PRIMARY);

    char rec_title[128];
    snprintf(rec_title, sizeof(rec_title), "Recommended Next: %s", app->progress.adaptive_recommended_topic);
    DrawText(rec_title, (int)(banner_rec.x + 68), (int)(banner_rec.y + 16), 16, COLOR_TEXT_PRIMARY);
    DrawText(app->progress.adaptive_recommendation_reason, (int)(banner_rec.x + 68), (int)(banner_rec.y + 42), 12, COLOR_TEXT_MUTED);

    Rectangle banner_btn = { banner_rec.x + banner_rec.width - 180.0f, banner_rec.y + 18.0f, 160.0f, 40.0f };
    if (ui_button(banner_btn, "Practice Now ->", BTN_PRIMARY, true)) {
        quiz_session_start(&app->quiz_session, &app->question_bank, app->progress.adaptive_recommended_topic, 180.0f);
        app->current_screen = SCREEN_QUIZ;
    }

    /* Topic Selection Cards (Requirement 5) */
    float topic_y = 290.0f;
    DrawText("Academic Practice Modules", (int)content_x, (int)topic_y, 18, COLOR_TEXT_PRIMARY);

    float topic_card_w = (content_w - 28.0f) / 3.0f;
    float topic_card_h = 220.0f;
    float topic_cards_start_y = topic_y + 30.0f;

    for (int t = 0; t < TOPIC_COUNT; t++) {
        Rectangle t_rec = { content_x + t * (topic_card_w + 14.0f), topic_cards_start_y, topic_card_w, topic_card_h };
        ui_card(t_rec, COLOR_CARD_BG, COLOR_BORDER, 0.14f);

        /* Topic Title & Badge */
        const char *t_name = topic_id_to_name((TopicID)t);
        DrawText(t_name, (int)(t_rec.x + 18), (int)(t_rec.y + 18), 17, COLOR_TEXT_PRIMARY);

        /* Question Count Badge */
        ui_badge((Vector2){ t_rec.x + 18, t_rec.y + 46 }, "6 Questions", (Color){ 30, 45, 74, 255 }, COLOR_PRIMARY);

        /* Description */
        const char *t_desc = topic_id_to_desc((TopicID)t);
        /* Word wrap simple description */
        DrawText(t_desc, (int)(t_rec.x + 18), (int)(t_rec.y + 78), 12, COLOR_TEXT_MUTED);

        /* Performance History line */
        const TopicPerformance *tp = &app->progress.topics[t];
        char perf_str[64];
        if (tp->has_attempts) {
            snprintf(perf_str, sizeof(perf_str), "Best: %d%% • %d attempts", tp->best_percentage, tp->attempts_count);
            DrawText(perf_str, (int)(t_rec.x + 18), (int)(t_rec.y + 138), 12, COLOR_SUCCESS);
        } else {
            DrawText("Status: Not attempted yet", (int)(t_rec.x + 18), (int)(t_rec.y + 138), 12, COLOR_TEXT_DARK);
        }

        /* Start Quiz Button */
        Rectangle start_btn = { t_rec.x + 18, t_rec.y + topic_card_h - 48, topic_card_w - 36, 36 };
        if (ui_button(start_btn, "Start Quiz", BTN_SECONDARY, true)) {
            quiz_session_start(&app->quiz_session, &app->question_bank, t_name, 180.0f);
            app->current_screen = SCREEN_QUIZ;
        }
    }

    /* Recent Activity Strip at Bottom */
    float recent_y = 560.0f;
    DrawText("Recent Quiz Activity", (int)content_x, (int)recent_y, 16, COLOR_TEXT_PRIMARY);

    Rectangle recent_rec = { content_x, recent_y + 24.0f, content_w, 140.0f };
    ui_card(recent_rec, COLOR_CARD_BG, COLOR_BORDER, 0.12f);

    int total_history = attempt_list_count(&app->attempt_history);
    if (total_history == 0) {
        DrawText("No quiz history recorded yet. Choose any topic above to take your first quiz!",
                 (int)(recent_rec.x + 24), (int)(recent_rec.y + 58), 13, COLOR_TEXT_MUTED);
    } else {
        /* Show up to 3 most recent attempts */
        int show_count = total_history > 3 ? 3 : total_history;
        float row_h = 36.0f;
        for (int i = 0; i < show_count; i++) {
            /* Most recent are at end of list */
            int attempt_idx = total_history - 1 - i;
            const QuizAttempt *a = attempt_list_get_at(&app->attempt_history, attempt_idx);
            if (!a) continue;

            float row_y = recent_rec.y + 14.0f + i * row_h;
            DrawText(a->topic_name, (int)(recent_rec.x + 20), (int)(row_y + 8), 14, COLOR_TEXT_PRIMARY);
            DrawText(a->timestamp, (int)(recent_rec.x + 240), (int)(row_y + 9), 12, COLOR_TEXT_MUTED);

            char score_str[32];
            snprintf(score_str, sizeof(score_str), "Score: %d/%d (%d%%)", a->correct_answers, a->total_questions, a->score_percentage);
            DrawText(score_str, (int)(recent_rec.x + 440), (int)(row_y + 8), 13, COLOR_TEXT_PRIMARY);

            Color cat_col = (a->score_percentage >= 80) ? COLOR_SUCCESS : ((a->score_percentage >= 50) ? COLOR_WARNING : COLOR_ERROR);
            const char *cat_name = (a->score_percentage >= 80) ? "Proficient" : ((a->score_percentage >= 50) ? "Developing" : "Needs Improvement");
            ui_badge((Vector2){ recent_rec.x + recent_rec.width - 160, row_y + 4 }, cat_name, (Color){ cat_col.r, cat_col.g, cat_col.b, 35 }, cat_col);

            if (i < show_count - 1) {
                DrawLine((int)(recent_rec.x + 16), (int)(row_y + row_h), (int)(recent_rec.x + recent_rec.width - 16), (int)(row_y + row_h), (Color){ 30, 42, 65, 100 });
            }
        }
    }
}

/* ========================================================================= */
/* SCREEN 3: QUIZ EXPERIENCE                                                 */
/* ========================================================================= */

static void screen_quiz_update_draw(AppState *app) {
    QuizSession *qs = &app->quiz_session;
    if (!qs->is_active || qs->total_questions == 0) {
        app->current_screen = SCREEN_DASHBOARD;
        return;
    }

    float pad = 48.0f;
    float top_y = 24.0f;

    /* Top Bar: Topic, Question Index, Timer, and Undo Action */
    DrawText(qs->topic, (int)pad, (int)top_y, 22, COLOR_TEXT_PRIMARY);

    char q_counter[64];
    snprintf(q_counter, sizeof(q_counter), "Question %d of %d", qs->current_index + 1, qs->total_questions);
    DrawText(q_counter, (int)pad, (int)(top_y + 28), 14, COLOR_TEXT_MUTED);

    /* Timer Display */
    int minutes = (int)(qs->time_remaining / 60.0f);
    int seconds = (int)(qs->time_remaining) % 60;
    char timer_str[32];
    snprintf(timer_str, sizeof(timer_str), "Time: %02d:%02d", minutes, seconds);
    Color timer_color = (qs->time_remaining < 30.0f) ? COLOR_ERROR : COLOR_WARNING;
    DrawText(timer_str, WINDOW_WIDTH - 240, (int)top_y + 4, 18, timer_color);

    /* Undo Last Answer Button (Stack-Based! Requirement 11.E) */
    bool can_undo = !stack_is_empty(&qs->undo_stack);
    Rectangle undo_btn = { WINDOW_WIDTH - 110, top_y, 80, 32 };
    if (ui_button(undo_btn, "Undo", BTN_OUTLINE, can_undo)) {
        quiz_session_undo_answer(qs);
    }

    /* Overall Quiz Progress Bar */
    float prog_fraction = (float)(qs->current_index + 1) / (float)qs->total_questions;
    Rectangle prog_rec = { pad, top_y + 56, WINDOW_WIDTH - pad * 2, 8 };
    ui_progress_bar(prog_rec, prog_fraction, COLOR_PRIMARY, (Color){ 25, 36, 60, 255 });

    /* Question Card */
    const Question *cur_q = &qs->active_questions[qs->current_index];
    float q_card_y = top_y + 78.0f;
    float q_card_h = 510.0f;
    Rectangle q_card = { pad, q_card_y, WINDOW_WIDTH - pad * 2, q_card_h };
    ui_card(q_card, COLOR_CARD_BG, COLOR_BORDER, 0.12f);

    /* Difficulty Badge */
    Color diff_col = COLOR_SUCCESS;
    if (strcmp(cur_q->difficulty, "Medium") == 0) diff_col = COLOR_WARNING;
    else if (strcmp(cur_q->difficulty, "Hard") == 0) diff_col = COLOR_ERROR;
    ui_badge((Vector2){ q_card.x + 32, q_card.y + 24 }, cur_q->difficulty, (Color){ diff_col.r, diff_col.g, diff_col.b, 35 }, diff_col);

    /* Question Text */
    DrawText(cur_q->text, (int)(q_card.x + 32), (int)(q_card.y + 60), 20, COLOR_TEXT_PRIMARY);

    /* 4 Answer Options */
    float opt_start_y = q_card.y + 120.0f;
    float opt_h = 56.0f;
    float opt_gap = 14.0f;
    int current_choice = qs->user_answers[qs->current_index];

    const char *option_prefixes[] = { "A", "B", "C", "D" };

    for (int opt = 0; opt < 4; opt++) {
        Rectangle opt_rec = { q_card.x + 32, opt_start_y + opt * (opt_h + opt_gap), q_card.width - 64, opt_h };
        bool is_selected = (current_choice == opt);

        Vector2 mouse_pos = GetMousePosition();
        bool hovered = CheckCollisionPointRec(mouse_pos, opt_rec);

        Color opt_bg = is_selected ? (Color){ 91, 140, 255, 35 } : (hovered ? COLOR_CARD_HOVER : (Color){ 17, 26, 46, 255 });
        Color opt_border = is_selected ? COLOR_PRIMARY : (hovered ? COLOR_BORDER_ACTIVE : COLOR_BORDER);

        DrawRectangleRounded(opt_rec, 0.2f, 6, opt_bg);
        DrawRectangleRoundedLines(opt_rec, 0.2f, 6, is_selected ? 1.8f : 1.2f, opt_border);

        /* Option Letter Circle Badge */
        Color badge_bg = is_selected ? COLOR_PRIMARY : (Color){ 30, 44, 70, 255 };
        Color badge_txt = is_selected ? (Color){ 10, 15, 30, 255 } : COLOR_TEXT_MUTED;
        DrawCircle((int)(opt_rec.x + 28), (int)(opt_rec.y + opt_h / 2), 14, badge_bg);
        DrawText(option_prefixes[opt], (int)(opt_rec.x + 23), (int)(opt_rec.y + opt_h / 2 - 7), 14, badge_txt);

        /* Option Text */
        Color txt_col = is_selected ? COLOR_PRIMARY : (hovered ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED);
        DrawText(cur_q->options[opt], (int)(opt_rec.x + 56), (int)(opt_rec.y + (opt_h - 16) / 2), 16, txt_col);

        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            quiz_session_select_answer(qs, opt);
        }
    }

    /* Question Navigator Dots at bottom of card */
    float dot_start_x = q_card.x + 32;
    for (int i = 0; i < qs->total_questions; i++) {
        Rectangle dot_rec = { dot_start_x + i * 36, q_card.y + q_card_h - 48, 28, 28 };
        bool is_curr = (i == qs->current_index);
        bool is_answered = (qs->user_answers[i] != -1);

        Color dot_bg = is_curr ? COLOR_PRIMARY : (is_answered ? (Color){ 52, 211, 153, 60 } : (Color){ 20, 30, 50, 255 });
        Color dot_border = is_curr ? COLOR_PRIMARY : (is_answered ? COLOR_SUCCESS : COLOR_BORDER);

        DrawRectangleRounded(dot_rec, 0.3f, 4, dot_bg);
        DrawRectangleRoundedLines(dot_rec, 0.3f, 4, 1.0f, dot_border);

        char num_s[8];
        snprintf(num_s, sizeof(num_s), "%d", i + 1);
        int num_w = MeasureText(num_s, 12);
        Color num_txt = is_curr ? (Color){ 10, 15, 30, 255 } : (is_answered ? COLOR_SUCCESS : COLOR_TEXT_MUTED);
        DrawText(num_s, (int)(dot_rec.x + (28 - num_w) / 2), (int)(dot_rec.y + 7), 12, num_txt);

        if (CheckCollisionPointRec(GetMousePosition(), dot_rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            quiz_session_jump_to(qs, i);
        }
    }

    /* Navigation Controls: Previous, Next, Submit */
    float btn_bar_y = q_card_y + q_card_h + 16.0f;
    float btn_w = 140.0f;
    float btn_h = 44.0f;

    bool has_prev = (qs->current_index > 0);
    Rectangle prev_btn = { pad, btn_bar_y, btn_w, btn_h };
    if (ui_button(prev_btn, "<- Previous", BTN_SECONDARY, has_prev)) {
        quiz_session_prev(qs);
    }

    bool is_last = (qs->current_index == qs->total_questions - 1);
    if (!is_last) {
        Rectangle next_btn = { WINDOW_WIDTH - pad - btn_w, btn_bar_y, btn_w, btn_h };
        if (ui_button(next_btn, "Next ->", BTN_PRIMARY, true)) {
            quiz_session_next(qs);
        }
    } else {
        Rectangle submit_btn = { WINDOW_WIDTH - pad - 160.0f, btn_bar_y, 160.0f, btn_h };
        if (ui_button(submit_btn, "Submit Quiz", BTN_SUCCESS, true)) {
            app->show_submit_modal = true;
        }
    }

    /* Additional early submit trigger */
    if (!is_last) {
        Rectangle early_submit = { WINDOW_WIDTH - pad - 300.0f, btn_bar_y, 140.0f, btn_h };
        if (ui_button(early_submit, "Submit Now", BTN_OUTLINE, true)) {
            app->show_submit_modal = true;
        }
    }

    /* Confirmation Modal Dialog */
    if (app->show_submit_modal) {
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){ 5, 8, 16, 200 });

        float modal_w = 440.0f;
        float modal_h = 240.0f;
        Rectangle modal_rec = { (WINDOW_WIDTH - modal_w) / 2.0f, (WINDOW_HEIGHT - modal_h) / 2.0f, modal_w, modal_h };
        ui_card(modal_rec, COLOR_CARD_BG, COLOR_PRIMARY, 0.15f);

        DrawText("Submit Quiz?", (int)(modal_rec.x + 32), (int)(modal_rec.y + 28), 20, COLOR_TEXT_PRIMARY);

        int answered_count = 0;
        for (int i = 0; i < qs->total_questions; i++) {
            if (qs->user_answers[i] != -1) answered_count++;
        }
        char confirm_msg[128];
        snprintf(confirm_msg, sizeof(confirm_msg), "You have answered %d of %d questions.", answered_count, qs->total_questions);
        DrawText(confirm_msg, (int)(modal_rec.x + 32), (int)(modal_rec.y + 64), 14, COLOR_TEXT_MUTED);
        DrawText("Are you sure you want to finish and submit your score?", (int)(modal_rec.x + 32), (int)(modal_rec.y + 88), 13, COLOR_TEXT_MUTED);

        Rectangle cancel_btn = { modal_rec.x + 32, modal_rec.y + modal_h - 60, 160, 40 };
        if (ui_button(cancel_btn, "Return to Quiz", BTN_SECONDARY, true)) {
            app->show_submit_modal = false;
        }

        Rectangle confirm_btn = { modal_rec.x + modal_w - 192, modal_rec.y + modal_h - 60, 160, 40 };
        if (ui_button(confirm_btn, "Confirm Submit", BTN_SUCCESS, true)) {
            app->show_submit_modal = false;
            quiz_session_submit(qs, &app->last_attempt, app->current_student.student_id);
            attempts_append_to_file(app->attempts_path, &app->last_attempt);
            app_refresh_data(app);
            app->current_screen = SCREEN_RESULTS;
        }
    }
}

/* ========================================================================= */
/* SCREEN 4: RESULTS SCREEN                                                  */
/* ========================================================================= */

static void screen_results_update_draw(AppState *app) {
    QuizAttempt *res = &app->last_attempt;
    float card_w = 640.0f;
    float card_h = 580.0f;
    Rectangle card_rec = { (WINDOW_WIDTH - card_w) / 2.0f, (WINDOW_HEIGHT - card_h) / 2.0f, card_w, card_h };
    ui_card(card_rec, COLOR_CARD_BG, COLOR_BORDER, 0.14f);

    /* Header */
    DrawText("Quiz Results", (int)(card_rec.x + 36), (int)(card_rec.y + 32), 24, COLOR_TEXT_PRIMARY);

    char topic_sub[128];
    snprintf(topic_sub, sizeof(topic_sub), "Topic: %s • Completed on %s", res->topic_name, res->timestamp);
    DrawText(topic_sub, (int)(card_rec.x + 36), (int)(card_rec.y + 64), 13, COLOR_TEXT_MUTED);

    /* Performance Category & Big Score Badge */
    Color cat_color = (res->score_percentage >= 80) ? COLOR_SUCCESS : ((res->score_percentage >= 50) ? COLOR_WARNING : COLOR_ERROR);
    const char *cat_title = (res->score_percentage >= 80) ? "Proficient" : ((res->score_percentage >= 50) ? "Developing" : "Needs Improvement");

    Rectangle score_banner = { card_rec.x + 36, card_rec.y + 96, card_w - 72, 110 };
    ui_card(score_banner, (Color){ cat_color.r, cat_color.g, cat_color.b, 25 }, cat_color, 0.16f);

    /* Big Percentage */
    char pct_str[16];
    snprintf(pct_str, sizeof(pct_str), "%d%%", res->score_percentage);
    DrawText(pct_str, (int)(score_banner.x + 32), (int)(score_banner.y + 24), 52, cat_color);

    /* Category Tag */
    ui_badge((Vector2){ score_banner.x + 190, score_banner.y + 28 }, cat_title, cat_color, (Color){ 10, 15, 30, 255 });

    char summary_msg[128];
    if (res->score_percentage >= 80) {
        snprintf(summary_msg, sizeof(summary_msg), "Outstanding! You demonstrated solid mastery of %s.", res->topic_name);
    } else if (res->score_percentage >= 50) {
        snprintf(summary_msg, sizeof(summary_msg), "Good effort! Practice more to cement core concepts in %s.", res->topic_name);
    } else {
        snprintf(summary_msg, sizeof(summary_msg), "Needs improvement. Review the answers and retry %s.", res->topic_name);
    }
    DrawText(summary_msg, (int)(score_banner.x + 190), (int)(score_banner.y + 62), 12, COLOR_TEXT_PRIMARY);

    /* 4 Metric Breakdown Cards */
    float stat_w = (card_w - 72.0f - 24.0f) / 3.0f;
    float stat_y = card_rec.y + 226.0f;
    float stat_h = 80.0f;

    char str_c[16], str_i[16], str_u[16];
    snprintf(str_c, sizeof(str_c), "%d", res->correct_answers);
    snprintf(str_i, sizeof(str_i), "%d", res->incorrect_answers);
    snprintf(str_u, sizeof(str_u), "%d", res->unanswered);

    ui_draw_stat_card((Rectangle){ card_rec.x + 36, stat_y, stat_w, stat_h }, "Correct", str_c, "Answers", COLOR_SUCCESS);
    ui_draw_stat_card((Rectangle){ card_rec.x + 36 + stat_w + 12.0f, stat_y, stat_w, stat_h }, "Incorrect", str_i, "Answers", COLOR_ERROR);
    ui_draw_stat_card((Rectangle){ card_rec.x + 36 + (stat_w + 12.0f) * 2, stat_y, stat_w, stat_h }, "Unanswered", str_u, "Skipped", COLOR_WARNING);

    /* Detailed score line */
    char detailed_msg[128];
    snprintf(detailed_msg, sizeof(detailed_msg), "Total Score: %d out of %d points recorded.", res->correct_answers, res->total_questions);
    DrawText(detailed_msg, (int)(card_rec.x + 36), (int)(card_rec.y + 330), 14, COLOR_TEXT_MUTED);

    /* Adaptive Advice Box */
    Rectangle advice_rec = { card_rec.x + 36, card_rec.y + 360, card_w - 72, 80 };
    ui_card(advice_rec, COLOR_BG_SECONDARY, COLOR_BORDER, 0.14f);
    DrawText("Upwise Adaptive Diagnostic:", (int)(advice_rec.x + 20), (int)(advice_rec.y + 16), 13, COLOR_PRIMARY);
    DrawText(app->progress.adaptive_recommendation_reason, (int)(advice_rec.x + 20), (int)(advice_rec.y + 38), 12, COLOR_TEXT_MUTED);

    /* Action Buttons */
    float btn_y = card_rec.y + card_h - 76.0f;
    float btn_w = (card_w - 72.0f - 24.0f) / 3.0f;

    Rectangle rev_btn = { card_rec.x + 36, btn_y, btn_w, 44 };
    if (ui_button(rev_btn, "Review Answers", BTN_PRIMARY, true)) {
        app->review_question_index = 0;
        app->current_screen = SCREEN_REVIEW;
    }

    Rectangle retry_btn = { card_rec.x + 36 + btn_w + 12.0f, btn_y, btn_w, 44 };
    if (ui_button(retry_btn, "Retry Quiz", BTN_SECONDARY, true)) {
        quiz_session_start(&app->quiz_session, &app->question_bank, res->topic_name, 180.0f);
        app->current_screen = SCREEN_QUIZ;
    }

    Rectangle dash_btn = { card_rec.x + 36 + (btn_w + 12.0f) * 2, btn_y, btn_w, 44 };
    if (ui_button(dash_btn, "Dashboard", BTN_OUTLINE, true)) {
        app->current_screen = SCREEN_DASHBOARD;
        app->active_nav_index = 0;
    }
}

/* ========================================================================= */
/* SCREEN 5: REVIEW ANSWERS                                                  */
/* ========================================================================= */

static void screen_review_update_draw(AppState *app) {
    QuizAttempt *att = &app->last_attempt;
    QuizSession *qs = &app->quiz_session;

    float pad = 48.0f;
    float top_y = 28.0f;

    /* Header */
    char rev_title[128];
    snprintf(rev_title, sizeof(rev_title), "Review Answers - %s", att->topic_name);
    DrawText(rev_title, (int)pad, (int)top_y, 22, COLOR_TEXT_PRIMARY);

    char rev_sub[128];
    snprintf(rev_sub, sizeof(rev_sub), "Question %d of %d • Attempt Score: %d%%",
             app->review_question_index + 1, att->total_questions, att->score_percentage);
    DrawText(rev_sub, (int)pad, (int)(top_y + 28), 13, COLOR_TEXT_MUTED);

    /* Back to Results Button */
    Rectangle back_btn = { WINDOW_WIDTH - pad - 160.0f, top_y, 160.0f, 36.0f };
    if (ui_button(back_btn, "<- Results Summary", BTN_OUTLINE, true)) {
        app->current_screen = SCREEN_RESULTS;
    }

    /* Review Card */
    float card_y = top_y + 70.0f;
    float card_h = 530.0f;
    Rectangle card_rec = { pad, card_y, WINDOW_WIDTH - pad * 2, card_h };
    ui_card(card_rec, COLOR_CARD_BG, COLOR_BORDER, 0.12f);

    int idx = app->review_question_index;
    const Question *q = &qs->active_questions[idx];
    int user_ans = att->user_answers[idx];
    int correct_ans = att->correct_answers_key[idx];

    /* Correctness Status Badge */
    if (user_ans == -1) {
        ui_badge((Vector2){ card_rec.x + 32, card_rec.y + 24 }, "UNANSWERED", (Color){ 251, 191, 36, 40 }, COLOR_WARNING);
    } else if (user_ans == correct_ans) {
        ui_badge((Vector2){ card_rec.x + 32, card_rec.y + 24 }, "CORRECT (+1)", (Color){ 52, 211, 153, 40 }, COLOR_SUCCESS);
    } else {
        ui_badge((Vector2){ card_rec.x + 32, card_rec.y + 24 }, "INCORRECT", (Color){ 251, 113, 133, 40 }, COLOR_ERROR);
    }

    /* Question Text */
    DrawText(q->text, (int)(card_rec.x + 32), (int)(card_rec.y + 60), 20, COLOR_TEXT_PRIMARY);

    /* 4 Options with status indicators */
    float opt_start_y = card_rec.y + 115.0f;
    float opt_h = 56.0f;
    float opt_gap = 14.0f;
    const char *prefixes[] = { "A", "B", "C", "D" };

    for (int opt = 0; opt < 4; opt++) {
        Rectangle opt_rec = { card_rec.x + 32, opt_start_y + opt * (opt_h + opt_gap), card_rec.width - 64, opt_h };
        bool is_correct = (opt == correct_ans);
        bool is_user_choice = (opt == user_ans);

        Color bg_col = COLOR_CARD_BG;
        Color border_col = COLOR_BORDER;

        if (is_correct) {
            bg_col = (Color){ 52, 211, 153, 25 };
            border_col = COLOR_SUCCESS;
        } else if (is_user_choice) {
            bg_col = (Color){ 251, 113, 133, 25 };
            border_col = COLOR_ERROR;
        }

        DrawRectangleRounded(opt_rec, 0.2f, 6, bg_col);
        DrawRectangleRoundedLines(opt_rec, 0.2f, 6, (is_correct || is_user_choice) ? 1.6f : 1.0f, border_col);

        /* Option Letter */
        DrawText(prefixes[opt], (int)(opt_rec.x + 24), (int)(opt_rec.y + (opt_h - 14) / 2), 14, COLOR_TEXT_MUTED);

        /* Option text */
        Color txt_col = is_correct ? COLOR_SUCCESS : (is_user_choice ? COLOR_ERROR : COLOR_TEXT_PRIMARY);
        DrawText(q->options[opt], (int)(opt_rec.x + 52), (int)(opt_rec.y + (opt_h - 16) / 2), 15, txt_col);

        /* Tag Label */
        if (is_correct && is_user_choice) {
            ui_badge((Vector2){ opt_rec.x + opt_rec.width - 150, opt_rec.y + 16 }, "Your Correct Choice", (Color){ 52, 211, 153, 50 }, COLOR_SUCCESS);
        } else if (is_correct) {
            ui_badge((Vector2){ opt_rec.x + opt_rec.width - 120, opt_rec.y + 16 }, "Correct Answer", (Color){ 52, 211, 153, 50 }, COLOR_SUCCESS);
        } else if (is_user_choice) {
            ui_badge((Vector2){ opt_rec.x + opt_rec.width - 130, opt_rec.y + 16 }, "Your Selection", (Color){ 251, 113, 133, 50 }, COLOR_ERROR);
        }
    }

    /* Explanation Note */
    DrawText("Review each answer key carefully to reinforce conceptual data structures understanding.",
             (int)(card_rec.x + 32), (int)(card_rec.y + card_h - 44), 12, COLOR_TEXT_DARK);

    /* Bottom Navigation Between Review Questions */
    float btn_bar_y = card_y + card_h + 16.0f;
    float btn_w = 140.0f;
    float btn_h = 42.0f;

    bool has_prev = (app->review_question_index > 0);
    Rectangle prev_btn = { pad, btn_bar_y, btn_w, btn_h };
    if (ui_button(prev_btn, "<- Previous Q", BTN_SECONDARY, has_prev)) {
        app->review_question_index--;
    }

    bool has_next = (app->review_question_index < att->total_questions - 1);
    Rectangle next_btn = { WINDOW_WIDTH - pad - btn_w, btn_bar_y, btn_w, btn_h };
    if (ui_button(next_btn, "Next Q ->", BTN_PRIMARY, has_next)) {
        app->review_question_index++;
    }
}

/* ========================================================================= */
/* SCREEN 6: MY PROGRESS                                                     */
/* ========================================================================= */

static void screen_progress_update_draw(AppState *app) {
    float content_x = SIDEBAR_WIDTH + 32.0f;
    float content_w = WINDOW_WIDTH - SIDEBAR_WIDTH - 64.0f;

    DrawText("Learning Analytics & Progress Tracking", (int)content_x, 30, 24, COLOR_TEXT_PRIMARY);
    DrawText("Historical diagnostic metrics derived from actual saved student quiz attempts.",
             (int)content_x, 62, 13, COLOR_TEXT_MUTED);

    /* 4 Stat Cards */
    float card_w = (content_w - 36.0f) / 4.0f;
    float stat_y = 92.0f;
    float stat_h = 88.0f;

    char str_quizzes[16];
    snprintf(str_quizzes, sizeof(str_quizzes), "%d", app->progress.total_quizzes);
    ui_draw_stat_card((Rectangle){ content_x, stat_y, card_w, stat_h }, "Quizzes Completed", str_quizzes, "Total sessions", COLOR_PRIMARY);

    char str_avg[16];
    snprintf(str_avg, sizeof(str_avg), "%d%%", app->progress.overall_average_percentage);
    ui_draw_stat_card((Rectangle){ content_x + card_w + 12.0f, stat_y, card_w, stat_h }, "Average Score", str_avg, "Historical average", COLOR_ACCENT);

    char str_best[16];
    snprintf(str_best, sizeof(str_best), "%d%%", app->progress.highest_score_percentage);
    ui_draw_stat_card((Rectangle){ content_x + (card_w + 12.0f) * 2, stat_y, card_w, stat_h }, "Highest Score", str_best, "Top milestone", COLOR_SUCCESS);

    char str_acc[16];
    snprintf(str_acc, sizeof(str_acc), "%d%%", app->progress.overall_accuracy_percentage);
    ui_draw_stat_card((Rectangle){ content_x + (card_w + 12.0f) * 3, stat_y, card_w, stat_h }, "Overall Accuracy", str_acc, "Questions solved", COLOR_WARNING);

    /* Topic-Wise Breakdown (Requirement 8) */
    float topic_sec_y = 196.0f;
    DrawText("Topic-Wise Mastery Breakdown", (int)content_x, (int)topic_sec_y, 18, COLOR_TEXT_PRIMARY);

    float topic_card_w = (content_w - 28.0f) / 3.0f;
    float topic_card_h = 180.0f;
    float topic_start_y = topic_sec_y + 28.0f;

    for (int t = 0; t < TOPIC_COUNT; t++) {
        Rectangle t_rec = { content_x + t * (topic_card_w + 14.0f), topic_start_y, topic_card_w, topic_card_h };
        ui_card(t_rec, COLOR_CARD_BG, COLOR_BORDER, 0.14f);

        const TopicPerformance *tp = &app->progress.topics[t];
        DrawText(tp->topic_name, (int)(t_rec.x + 18), (int)(t_rec.y + 16), 16, COLOR_TEXT_PRIMARY);

        Color cat_col = (tp->average_percentage >= 80) ? COLOR_SUCCESS : ((tp->average_percentage >= 50) ? COLOR_WARNING : COLOR_ERROR);
        if (!tp->has_attempts) cat_col = COLOR_TEXT_DARK;
        ui_badge((Vector2){ t_rec.x + 18, t_rec.y + 44 }, tp->category_label, (Color){ cat_col.r, cat_col.g, cat_col.b, 35 }, cat_col);

        /* Progress Bar */
        float frac = tp->average_percentage / 100.0f;
        Rectangle p_rec = { t_rec.x + 18, t_rec.y + 80, topic_card_w - 36, 8 };
        ui_progress_bar(p_rec, frac, cat_col, (Color){ 20, 30, 48, 255 });

        /* Metrics */
        char stat_line[64];
        snprintf(stat_line, sizeof(stat_line), "Attempts: %d  •  Avg: %d%%", tp->attempts_count, tp->average_percentage);
        DrawText(stat_line, (int)(t_rec.x + 18), (int)(t_rec.y + 104), 12, COLOR_TEXT_MUTED);

        char best_line[64];
        if (tp->has_attempts) {
            snprintf(best_line, sizeof(best_line), "Best: %d%%  •  Latest: %d%%", tp->best_percentage, tp->latest_percentage);
        } else {
            snprintf(best_line, sizeof(best_line), "No recorded attempts");
        }
        DrawText(best_line, (int)(t_rec.x + 18), (int)(t_rec.y + 128), 12, COLOR_TEXT_MUTED);
    }

    /* Strongest & Weakest Topic Callout & Score Trend Chart */
    float chart_y = 424.0f;
    float chart_w = content_w * 0.65f;
    float chart_h = 300.0f;

    /* Extract recent 7 attempts into an array for the bar chart */
    int recent_scores[7];
    const char *recent_labels[7];
    int count = 0;
    int total_history = attempt_list_count(&app->attempt_history);
    int start_idx = total_history > 7 ? total_history - 7 : 0;

    for (int i = start_idx; i < total_history && count < 7; i++) {
        const QuizAttempt *a = attempt_list_get_at(&app->attempt_history, i);
        if (a) {
            recent_scores[count] = a->score_percentage;
            /* Label as C, DS, CF */
            if (strstr(a->topic_name, "C Program")) recent_labels[count] = "C";
            else if (strstr(a->topic_name, "Data")) recent_labels[count] = "DS";
            else recent_labels[count] = "CF";
            count++;
        }
    }

    ui_draw_score_barchart((Rectangle){ content_x, chart_y, chart_w, chart_h }, recent_scores, count, 100, recent_labels);

    /* Right Side Analysis Panel */
    float panel_x = content_x + chart_w + 16.0f;
    float panel_w = content_w - chart_w - 16.0f;
    Rectangle panel_rec = { panel_x, chart_y, panel_w, chart_h };
    ui_card(panel_rec, COLOR_CARD_BG, COLOR_BORDER, 0.14f);

    DrawText("Academic Analysis", (int)(panel_rec.x + 20), (int)(panel_rec.y + 18), 16, COLOR_TEXT_PRIMARY);

    /* Strongest Topic Card */
    Rectangle strong_rec = { panel_rec.x + 16, panel_rec.y + 54, panel_w - 32, 70 };
    ui_card(strong_rec, (Color){ 52, 211, 153, 20 }, COLOR_SUCCESS, 0.15f);
    DrawText("Strongest Topic", (int)(strong_rec.x + 16), (int)(strong_rec.y + 12), 12, COLOR_SUCCESS);
    if (app->progress.strongest_topic_index >= 0) {
        DrawText(app->progress.topics[app->progress.strongest_topic_index].topic_name,
                 (int)(strong_rec.x + 16), (int)(strong_rec.y + 32), 15, COLOR_TEXT_PRIMARY);
    } else {
        DrawText("None yet", (int)(strong_rec.x + 16), (int)(strong_rec.y + 32), 15, COLOR_TEXT_DARK);
    }

    /* Weakest Topic Card */
    Rectangle weak_rec = { panel_rec.x + 16, panel_rec.y + 134, panel_w - 32, 70 };
    ui_card(weak_rec, (Color){ 251, 113, 133, 20 }, COLOR_ERROR, 0.15f);
    DrawText("Weakest Topic", (int)(weak_rec.x + 16), (int)(weak_rec.y + 12), 12, COLOR_ERROR);
    if (app->progress.weakest_topic_index >= 0) {
        DrawText(app->progress.topics[app->progress.weakest_topic_index].topic_name,
                 (int)(weak_rec.x + 16), (int)(weak_rec.y + 32), 15, COLOR_TEXT_PRIMARY);
    } else {
        DrawText("None yet", (int)(weak_rec.x + 16), (int)(weak_rec.y + 32), 15, COLOR_TEXT_DARK);
    }

    /* Adaptive Practice Call-To-Action */
    Rectangle cta_rec = { panel_rec.x + 16, panel_rec.y + 214, panel_w - 32, 68 };
    ui_card(cta_rec, (Color){ 91, 140, 255, 20 }, COLOR_PRIMARY, 0.15f);
    DrawText("Practice Recommended", (int)(cta_rec.x + 16), (int)(cta_rec.y + 10), 12, COLOR_PRIMARY);
    DrawText(app->progress.adaptive_recommended_topic, (int)(cta_rec.x + 16), (int)(cta_rec.y + 28), 14, COLOR_TEXT_PRIMARY);
}

/* ========================================================================= */
/* SCREEN 7: LEADERBOARD                                                     */
/* ========================================================================= */

static void screen_leaderboard_update_draw(AppState *app) {
    float content_x = SIDEBAR_WIDTH + 32.0f;
    float content_w = WINDOW_WIDTH - SIDEBAR_WIDTH - 64.0f;

    DrawText("Student Cohort Leaderboard", (int)content_x, 30, 24, COLOR_TEXT_PRIMARY);
    DrawText("Live rankings sorted via Quicksort in C based on average accuracy and completed practice sessions.",
             (int)content_x, 62, 13, COLOR_TEXT_MUTED);

    /* Search by Student ID (Requirement 9 & 11.F) */
    float search_y = 94.0f;
    Rectangle search_rec = { content_x, search_y, 320.0f, 40.0f };
    ui_text_input(search_rec, &app->leaderboard_search_input, "Search by Student ID (e.g. CS-2024)");

    int matched_idx = -1;
    if (strlen(app->leaderboard_search_input.text) > 0) {
        matched_idx = leaderboard_search_by_id(&app->leaderboard, app->leaderboard_search_input.text);
    }

    /* Leaderboard Table Container */
    float table_y = 150.0f;
    float table_h = 560.0f;
    Rectangle table_rec = { content_x, table_y, content_w, table_h };
    ui_card(table_rec, COLOR_CARD_BG, COLOR_BORDER, 0.12f);

    /* Table Header */
    float head_y = table_rec.y + 16.0f;
    DrawText("RANK", (int)(table_rec.x + 28), (int)head_y, 12, COLOR_TEXT_MUTED);
    DrawText("STUDENT NAME", (int)(table_rec.x + 100), (int)head_y, 12, COLOR_TEXT_MUTED);
    DrawText("STUDENT ID", (int)(table_rec.x + 360), (int)head_y, 12, COLOR_TEXT_MUTED);
    DrawText("AVERAGE SCORE", (int)(table_rec.x + 550), (int)head_y, 12, COLOR_TEXT_MUTED);
    DrawText("QUIZZES COMPLETED", (int)(table_rec.x + 750), (int)head_y, 12, COLOR_TEXT_MUTED);

    DrawLine((int)(table_rec.x + 16), (int)(head_y + 24), (int)(table_rec.x + table_rec.width - 16), (int)(head_y + 24), COLOR_BORDER);

    /* Table Rows */
    float row_start_y = head_y + 36.0f;
    float row_h = 46.0f;

    for (int i = 0; i < app->leaderboard.count && i < 10; i++) {
        const LeaderboardEntry *e = &app->leaderboard.entries[i];
        float cur_row_y = row_start_y + i * row_h;

        bool is_search_hit = (i == matched_idx);
        bool is_user = e->is_current_student;

        if (is_search_hit) {
            DrawRectangleRec((Rectangle){ table_rec.x + 8, cur_row_y - 4, table_rec.width - 16, row_h }, (Color){ 91, 140, 255, 45 });
        } else if (is_user) {
            DrawRectangleRec((Rectangle){ table_rec.x + 8, cur_row_y - 4, table_rec.width - 16, row_h }, (Color){ 139, 92, 246, 30 });
        }

        /* Rank Badge */
        Color rank_col = COLOR_TEXT_PRIMARY;
        if (e->rank == 1) rank_col = COLOR_WARNING;      /* Gold */
        else if (e->rank == 2) rank_col = (Color){ 200, 210, 225, 255 }; /* Silver */
        else if (e->rank == 3) rank_col = (Color){ 205, 127, 50, 255 };  /* Bronze */

        char rank_str[8];
        snprintf(rank_str, sizeof(rank_str), "#%d", e->rank);
        DrawText(rank_str, (int)(table_rec.x + 30), (int)(cur_row_y + 8), 14, rank_col);

        /* Name */
        DrawText(e->name, (int)(table_rec.x + 100), (int)(cur_row_y + 8), 14, COLOR_TEXT_PRIMARY);
        if (is_user) {
            ui_badge((Vector2){ table_rec.x + 270, cur_row_y + 4 }, "YOU", (Color){ 139, 92, 246, 60 }, COLOR_ACCENT);
        }

        /* Student ID */
        DrawText(e->student_id, (int)(table_rec.x + 360), (int)(cur_row_y + 8), 13, COLOR_TEXT_MUTED);

        /* Average Score Bar & Text */
        Rectangle avg_bar = { table_rec.x + 550, cur_row_y + 12, 100, 8 };
        Color bar_c = (e->average_percentage >= 80) ? COLOR_SUCCESS : ((e->average_percentage >= 50) ? COLOR_WARNING : COLOR_ERROR);
        ui_progress_bar(avg_bar, e->average_percentage / 100.0f, bar_c, (Color){ 20, 30, 48, 255 });

        char avg_txt[16];
        snprintf(avg_txt, sizeof(avg_txt), "%d%%", e->average_percentage);
        DrawText(avg_txt, (int)(table_rec.x + 660), (int)(cur_row_y + 8), 13, COLOR_TEXT_PRIMARY);

        /* Quizzes Completed */
        char q_str[16];
        snprintf(q_str, sizeof(q_str), "%d", e->quizzes_completed);
        DrawText(q_str, (int)(table_rec.x + 790), (int)(cur_row_y + 8), 13, COLOR_TEXT_PRIMARY);

        DrawLine((int)(table_rec.x + 16), (int)(cur_row_y + row_h - 6), (int)(table_rec.x + table_rec.width - 16), (int)(cur_row_y + row_h - 6), (Color){ 30, 42, 65, 80 });
    }
}

/* ========================================================================= */
/* SCREEN 8: PROFILE & SETTINGS                                              */
/* ========================================================================= */

static void screen_settings_update_draw(AppState *app) {
    float content_x = SIDEBAR_WIDTH + 32.0f;
    float content_w = WINDOW_WIDTH - SIDEBAR_WIDTH - 64.0f;

    DrawText("Profile & System Settings", (int)content_x, 30, 24, COLOR_TEXT_PRIMARY);
    DrawText("Update your student identity or manage local attempt records.", (int)content_x, 62, 13, COLOR_TEXT_MUTED);

    float card_w = 520.0f;
    float card_h = 360.0f;
    Rectangle card_rec = { content_x, 96.0f, card_w, card_h };
    ui_card(card_rec, COLOR_CARD_BG, COLOR_BORDER, 0.14f);

    DrawText("Student Identity", (int)(card_rec.x + 36), (int)(card_rec.y + 28), 18, COLOR_TEXT_PRIMARY);

    DrawText("Full Name", (int)(card_rec.x + 36), (int)(card_rec.y + 64), 13, COLOR_TEXT_PRIMARY);
    Rectangle name_rec = { card_rec.x + 36, card_rec.y + 86, card_w - 72, 42 };
    ui_text_input(name_rec, &app->name_input, "Student Full Name");

    DrawText("Student ID", (int)(card_rec.x + 36), (int)(card_rec.y + 140), 13, COLOR_TEXT_PRIMARY);
    Rectangle id_rec = { card_rec.x + 36, card_rec.y + 162, card_w - 72, 42 };
    ui_text_input(id_rec, &app->id_input, "Student ID");

    if (strlen(app->validation_error) > 0) {
        DrawText(app->validation_error, (int)(card_rec.x + 36), (int)(card_rec.y + 216), 12, COLOR_ERROR);
    }

    Rectangle save_btn = { card_rec.x + 36, card_rec.y + 246, 180, 44 };
    if (ui_button(save_btn, "Save Changes", BTN_PRIMARY, true)) {
        char err[128] = {0};
        if (student_validate(app->name_input.text, app->id_input.text, err, sizeof(err))) {
            strncpy(app->current_student.name, app->name_input.text, sizeof(app->current_student.name) - 1);
            strncpy(app->current_student.student_id, app->id_input.text, sizeof(app->current_student.student_id) - 1);
            app->current_student.is_registered = true;

            student_save_profile(app->profile_path, &app->current_student);
            app_refresh_data(app);
            app->validation_error[0] = '\0';
        } else {
            strncpy(app->validation_error, err, sizeof(app->validation_error) - 1);
        }
    }

    /* Right Side Storage Info Card */
    float info_x = content_x + card_w + 24.0f;
    float info_w = content_w - card_w - 24.0f;
    Rectangle info_rec = { info_x, 96.0f, info_w, card_h };
    ui_card(info_rec, COLOR_CARD_BG, COLOR_BORDER, 0.14f);

    DrawText("Data Structures & Storage", (int)(info_rec.x + 24), (int)(info_rec.y + 28), 18, COLOR_TEXT_PRIMARY);
    DrawText("• Question Queue: Queue (FIFO) in data_structures.c\n"
             "• Undo History: Stack (LIFO) for answer undo\n"
             "• Attempts History: Linked List dynamic traversal\n"
             "• Leaderboard: Quicksort algorithm in C\n"
             "• Search Strategy: Linear ID lookup algorithm\n\n"
             "Storage Files:\n"
             "• data/students.txt\n"
             "• data/questions.txt\n"
             "• data/attempts.txt\n"
             "• data/leaderboard.txt",
             (int)(info_rec.x + 24), (int)(info_rec.y + 68), 13, COLOR_TEXT_MUTED);
}
