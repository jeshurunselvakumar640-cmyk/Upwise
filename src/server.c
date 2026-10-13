#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "student.h"
#include "quiz.h"
#include "progress.h"
#include "leaderboard.h"
#include "data_structures.h"

#define PORT 8080
#define BUFFER_SIZE 65536

static const char *STUDENT_FILE = "data/students.txt";
static const char *QUESTIONS_FILE = "data/questions.txt";
static const char *ATTEMPTS_FILE = "data/attempts.txt";
static const char *LEADERBOARD_FILE = "data/leaderboard.txt";

static void send_response(SOCKET client, const char *status, const char *content_type, const char *body, int body_len) {
    char header[512];
    snprintf(header, sizeof(header),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s; charset=UTF-8\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "\r\n",
        status, content_type, body_len);

    send(client, header, strlen(header), 0);
    if (body && body_len > 0) {
        send(client, body, body_len, 0);
    }
}

static void serve_file(SOCKET client, const char *filepath, const char *content_type) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        const char *not_found = "404 Not Found";
        send_response(client, "404 Not Found", "text/plain", not_found, strlen(not_found));
        return;
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buf = (char *)malloc(size + 1);
    if (!buf) {
        fclose(fp);
        const char *err = "500 Internal Error";
        send_response(client, "500 Internal Server Error", "text/plain", err, strlen(err));
        return;
    }
    fread(buf, 1, size, fp);
    fclose(fp);

    send_response(client, "200 OK", content_type, buf, size);
    free(buf);
}

static void handle_get_profile(SOCKET client) {
    Student s;
    student_init(&s);
    bool ok = student_load_profile(STUDENT_FILE, &s);
    char json[256];
    if (ok && s.is_registered) {
        snprintf(json, sizeof(json),
            "{\"name\":\"%s\",\"student_id\":\"%s\",\"is_registered\":true}",
            s.name, s.student_id);
    } else {
        snprintf(json, sizeof(json), "{\"is_registered\":false}");
    }
    send_response(client, "200 OK", "application/json", json, strlen(json));
}

static void handle_get_questions(SOCKET client, const char *query) {
    QuestionBank bank;
    question_bank_init(&bank);
    question_bank_load_from_file(&bank, QUESTIONS_FILE);

    char filter_topic[64] = {0};
    if (query && strstr(query, "topic=")) {
        const char *t_param = strstr(query, "topic=") + 6;
        char decoded[64] = {0};
        int di = 0;
        for (int i = 0; t_param[i] && t_param[i] != '&' && di < 63; i++) {
            if (t_param[i] == '%' && t_param[i+1] && t_param[i+2]) {
                int hex = 0;
                sscanf(&t_param[i+1], "%2x", &hex);
                decoded[di++] = (char)hex;
                i += 2;
            } else if (t_param[i] == '+') {
                decoded[di++] = ' ';
            } else {
                decoded[di++] = t_param[i];
            }
        }
        decoded[di] = '\0';
        strncpy(filter_topic, decoded, sizeof(filter_topic) - 1);
    }

    char *json = (char *)malloc(BUFFER_SIZE);
    strcpy(json, "[");
    bool first = true;

    for (int i = 0; i < bank.count; i++) {
        Question *q = &bank.questions[i];
        if (strlen(filter_topic) > 0 && strcmp(q->topic, filter_topic) != 0) {
            continue;
        }

        char item[1024];
        snprintf(item, sizeof(item),
            "%s{\"id\":%d,\"topic\":\"%s\",\"text\":\"%s\",\"options\":[\"%s\",\"%s\",\"%s\",\"%s\"],\"correct_index\":%d,\"difficulty\":\"%s\"}",
            first ? "" : ",",
            q->id, q->topic, q->text,
            q->options[0], q->options[1], q->options[2], q->options[3],
            q->correct_index, q->difficulty);

        strcat(json, item);
        first = false;
    }
    strcat(json, "]");

    send_response(client, "200 OK", "application/json", json, strlen(json));
    free(json);
}

static void handle_get_progress(SOCKET client) {
    Student s;
    student_init(&s);
    student_load_profile(STUDENT_FILE, &s);

    AttemptList list;
    attempt_list_init(&list);
    attempts_load_from_file(&list, ATTEMPTS_FILE, s.student_id);

    OverallProgress prog;
    progress_calculate(&list, &prog);

    char *json = (char *)malloc(BUFFER_SIZE);
    char buf[512];

    snprintf(json, BUFFER_SIZE,
        "{\"total_quizzes\":%d,\"overall_average_percentage\":%d,\"highest_score_percentage\":%d,\"overall_accuracy_percentage\":%d,"
        "\"adaptive_recommended_topic\":\"%s\",\"adaptive_recommendation_reason\":\"%s\","
        "\"strongest_topic\":\"%s\",\"weakest_topic\":\"%s\",\"topics\":[",
        prog.total_quizzes, prog.overall_average_percentage, prog.highest_score_percentage, prog.overall_accuracy_percentage,
        prog.adaptive_recommended_topic, prog.adaptive_recommendation_reason,
        (prog.strongest_topic_index >= 0) ? prog.topics[prog.strongest_topic_index].topic_name : "None",
        (prog.weakest_topic_index >= 0) ? prog.topics[prog.weakest_topic_index].topic_name : "None");

    for (int t = 0; t < TOPIC_COUNT; t++) {
        TopicPerformance *tp = &prog.topics[t];
        snprintf(buf, sizeof(buf),
            "%s{\"topic_name\":\"%s\",\"attempts_count\":%d,\"average_percentage\":%d,\"best_percentage\":%d,\"latest_percentage\":%d,\"category_label\":\"%s\",\"has_attempts\":%s}",
            (t == 0) ? "" : ",",
            tp->topic_name, tp->attempts_count, tp->average_percentage, tp->best_percentage, tp->latest_percentage,
            tp->category_label, tp->has_attempts ? "true" : "false");
        strcat(json, buf);
    }
    strcat(json, "],\"recent_attempts\":[");

    int total_history = attempt_list_count(&list);
    int show = total_history > 5 ? 5 : total_history;
    for (int i = 0; i < show; i++) {
        const QuizAttempt *a = attempt_list_get_at(&list, total_history - 1 - i);
        if (!a) continue;
        snprintf(buf, sizeof(buf),
            "%s{\"topic_name\":\"%s\",\"score_percentage\":%d,\"timestamp\":\"%s\"}",
            (i == 0) ? "" : ",", a->topic_name, a->score_percentage, a->timestamp);
        strcat(json, buf);
    }
    strcat(json, "]}");

    send_response(client, "200 OK", "application/json", json, strlen(json));
    free(json);
    attempt_list_free(&list);
}

static void handle_get_leaderboard(SOCKET client) {
    Student s;
    student_init(&s);
    student_load_profile(STUDENT_FILE, &s);

    AttemptList list;
    attempt_list_init(&list);
    attempts_load_from_file(&list, ATTEMPTS_FILE, s.student_id);

    OverallProgress prog;
    progress_calculate(&list, &prog);

    Leaderboard lb;
    leaderboard_load_or_seed(&lb, LEADERBOARD_FILE);
    leaderboard_update_current_student(&lb, &s, &prog);
    leaderboard_save_to_file(&lb, LEADERBOARD_FILE);

    char *json = (char *)malloc(BUFFER_SIZE);
    strcpy(json, "[");
    for (int i = 0; i < lb.count; i++) {
        LeaderboardEntry *e = &lb.entries[i];
        char row[512];
        snprintf(row, sizeof(row),
            "%s{\"rank\":%d,\"name\":\"%s\",\"student_id\":\"%s\",\"average_percentage\":%d,\"quizzes_completed\":%d,\"is_current_student\":%s}",
            (i == 0) ? "" : ",",
            e->rank, e->name, e->student_id, e->average_percentage, e->quizzes_completed,
            e->is_current_student ? "true" : "false");
        strcat(json, row);
    }
    strcat(json, "]");

    send_response(client, "200 OK", "application/json", json, strlen(json));
    free(json);
    attempt_list_free(&list);
}

static void handle_post_profile(SOCKET client, const char *body) {
    char name[64] = {0};
    char sid[32] = {0};

    const char *p_name = strstr(body, "\"name\":\"");
    if (p_name) {
        sscanf(p_name + 8, "%[^\"]", name);
    }
    const char *p_sid = strstr(body, "\"student_id\":\"");
    if (p_sid) {
        sscanf(p_sid + 14, "%[^\"]", sid);
    }

    if (strlen(name) > 0 && strlen(sid) > 0) {
        Student s;
        strncpy(s.name, name, sizeof(s.name) - 1);
        strncpy(s.student_id, sid, sizeof(s.student_id) - 1);
        s.is_registered = true;
        student_save_profile(STUDENT_FILE, &s);

        const char *ok_resp = "{\"success\":true}";
        send_response(client, "200 OK", "application/json", ok_resp, strlen(ok_resp));
    } else {
        const char *fail_resp = "{\"success\":false,\"error\":\"Invalid name or ID\"}";
        send_response(client, "400 Bad Request", "application/json", fail_resp, strlen(fail_resp));
    }
}

static void handle_post_submit(SOCKET client, const char *body) {
    Student s;
    student_init(&s);
    student_load_profile(STUDENT_FILE, &s);

    QuizAttempt a;
    memset(&a, 0, sizeof(QuizAttempt));
    time_t now = time(NULL);
    snprintf(a.attempt_id, sizeof(a.attempt_id), "ATT-%ld", (long)now);
    strncpy(a.student_id, s.is_registered ? s.student_id : "UNKNOWN", sizeof(a.student_id) - 1);

    const char *p_top = strstr(body, "\"topic\":\"");
    if (p_top) {
        sscanf(p_top + 9, "%[^\"]", a.topic_name);
    } else {
        strcpy(a.topic_name, "C Programming");
    }

    struct tm *t = localtime(&now);
    if (t) strftime(a.timestamp, sizeof(a.timestamp), "%Y-%m-%d %H:%M", t);
    else strcpy(a.timestamp, "2026-10-13 12:00");

    /* Parse answers */
    int user_ans[MAX_QUIZ_QUESTIONS] = {0};
    int q_ids[MAX_QUIZ_QUESTIONS] = {0};
    int corr_keys[MAX_QUIZ_QUESTIONS] = {0};
    int count = 0;

    const char *p_ans = strstr(body, "\"answers\":[");
    if (p_ans) {
        const char *cur = p_ans + 11;
        while (*cur && *cur != ']' && count < MAX_QUIZ_QUESTIONS) {
            user_ans[count++] = atoi(cur);
            while (*cur && *cur != ',' && *cur != ']') cur++;
            if (*cur == ',') cur++;
        }
    }
    a.total_questions = count;

    int q_count = 0;
    const char *p_qid = strstr(body, "\"question_ids\":[");
    if (p_qid) {
        const char *cur = p_qid + 16;
        while (*cur && *cur != ']' && q_count < MAX_QUIZ_QUESTIONS) {
            q_ids[q_count++] = atoi(cur);
            while (*cur && *cur != ',' && *cur != ']') cur++;
            if (*cur == ',') cur++;
        }
    }

    int k_count = 0;
    const char *p_key = strstr(body, "\"correct_keys\":[");
    if (p_key) {
        const char *cur = p_key + 16;
        while (*cur && *cur != ']' && k_count < MAX_QUIZ_QUESTIONS) {
            corr_keys[k_count++] = atoi(cur);
            while (*cur && *cur != ',' && *cur != ']') cur++;
            if (*cur == ',') cur++;
        }
    }

    for (int i = 0; i < a.total_questions; i++) {
        a.question_ids[i] = q_ids[i];
        a.user_answers[i] = user_ans[i];
        a.correct_answers_key[i] = corr_keys[i];

        if (user_ans[i] == -1) {
            a.unanswered++;
        } else if (user_ans[i] == corr_keys[i]) {
            a.correct_answers++;
        } else {
            a.incorrect_answers++;
        }
    }

    if (a.total_questions > 0) {
        a.score_percentage = (a.correct_answers * 100) / a.total_questions;
    }

    /* Append attempt to storage */
    attempts_append_to_file(ATTEMPTS_FILE, &a);

    /* Return attempt result */
    const char *cat = (a.score_percentage >= 80) ? "Proficient" : ((a.score_percentage >= 50) ? "Developing" : "Needs Improvement");
    char resp[1024];
    snprintf(resp, sizeof(resp),
        "{\"attempt_id\":\"%s\",\"topic_name\":\"%s\",\"total_questions\":%d,"
        "\"correct_answers\":%d,\"incorrect_answers\":%d,\"unanswered\":%d,"
        "\"score_percentage\":%d,\"performance_category\":\"%s\",\"timestamp\":\"%s\","
        "\"user_answers\":[%d,%d,%d,%d,%d,%d],\"correct_answers_key\":[%d,%d,%d,%d,%d,%d]}",
        a.attempt_id, a.topic_name, a.total_questions,
        a.correct_answers, a.incorrect_answers, a.unanswered,
        a.score_percentage, cat, a.timestamp,
        a.user_answers[0], a.user_answers[1], a.user_answers[2], a.user_answers[3], a.user_answers[4], a.user_answers[5],
        a.correct_answers_key[0], a.correct_answers_key[1], a.correct_answers_key[2], a.correct_answers_key[3], a.correct_answers_key[4], a.correct_answers_key[5]);

    send_response(client, "200 OK", "application/json", resp, strlen(resp));
}

int main(void) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("[ERROR] WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    SOCKET server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        printf("[ERROR] Socket creation failed\n");
        WSACleanup();
        return 1;
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("[ERROR] Port %d is busy. Another Upwise instance may already be running.\n", PORT);
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    if (listen(server_sock, 10) == SOCKET_ERROR) {
        printf("[ERROR] Listen failed\n");
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    printf("=========================================================\n");
    printf("        UPWISE LOCALHOST WEB SERVER RUNNING             \n");
    printf("              Learn. Practise. Progress.                 \n");
    printf("=========================================================\n");
    printf(" [*] Serving Upwise at: http://localhost:%d\n", PORT);
    printf(" [*] Press Ctrl+C in this console to stop server.\n");
    printf("=========================================================\n");

    /* Open default web browser */
    ShellExecute(NULL, "open", "http://localhost:8080", NULL, NULL, SW_SHOWNORMAL);

    while (1) {
        struct sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        SOCKET client = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client == INVALID_SOCKET) continue;

        char buffer[BUFFER_SIZE];
        int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            closesocket(client);
            continue;
        }
        buffer[bytes] = '\0';

        char method[16] = {0};
        char path[256] = {0};
        sscanf(buffer, "%15s %255s", method, path);

        /* Route handling */
        if (strcmp(method, "OPTIONS") == 0) {
            send_response(client, "200 OK", "text/plain", "", 0);
        } else if (strcmp(method, "GET") == 0) {
            if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
                serve_file(client, "web/index.html", "text/html");
            } else if (strcmp(path, "/style.css") == 0) {
                serve_file(client, "web/style.css", "text/css");
            } else if (strcmp(path, "/app.js") == 0) {
                serve_file(client, "web/app.js", "application/javascript");
            } else if (strcmp(path, "/api/profile") == 0) {
                handle_get_profile(client);
            } else if (strncmp(path, "/api/questions", 14) == 0) {
                const char *q = strchr(path, '?');
                handle_get_questions(client, q ? q + 1 : NULL);
            } else if (strcmp(path, "/api/progress") == 0) {
                handle_get_progress(client);
            } else if (strcmp(path, "/api/leaderboard") == 0) {
                handle_get_leaderboard(client);
            } else {
                serve_file(client, "web/index.html", "text/html");
            }
        } else if (strcmp(method, "POST") == 0) {
            const char *body = strstr(buffer, "\r\n\r\n");
            if (body) body += 4;
            else body = "";

            if (strcmp(path, "/api/profile") == 0) {
                handle_post_profile(client, body);
            } else if (strcmp(path, "/api/submit") == 0) {
                handle_post_submit(client, body);
            } else {
                const char *not_found = "404 Not Found";
                send_response(client, "404 Not Found", "text/plain", not_found, strlen(not_found));
            }
        }

        closesocket(client);
    }

    closesocket(server_sock);
    WSACleanup();
    return 0;
}
