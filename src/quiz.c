#include "quiz.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char* topic_id_to_name(TopicID id) {
    switch (id) {
        case TOPIC_C_PROGRAMMING: return "C Programming";
        case TOPIC_DATA_STRUCTURES: return "Data Structures";
        case TOPIC_COMPUTER_FUNDAMENTALS: return "Computer Fundamentals";
        default: return "General";
    }
}

const char* topic_id_to_desc(TopicID id) {
    switch (id) {
        case TOPIC_C_PROGRAMMING:
            return "Test pointers, memory management, syntax, and standard C library fundamentals.";
        case TOPIC_DATA_STRUCTURES:
            return "Sharpen your knowledge of stacks, queues, trees, linked lists, and algorithm complexities.";
        case TOPIC_COMPUTER_FUNDAMENTALS:
            return "Master core computer hardware, memory architectures, number systems, and OS concepts.";
        default:
            return "Academic topics for comprehensive learning.";
    }
}

void question_bank_init(QuestionBank *bank) {
    if (!bank) return;
    memset(bank, 0, sizeof(QuestionBank));
    bank->count = 0;
}

void question_bank_load_defaults(QuestionBank *bank) {
    if (!bank) return;
    bank->count = 0;

    /* C Programming Questions */
    Question c_q[] = {
        {
            101, "C Programming",
            "Which format specifier is standard for printing a memory address in C?",
            {"%x", "%p", "%u", "%d"},
            1, "Easy"
        },
        {
            102, "C Programming",
            "What is the return type of the malloc() function in C?",
            {"int*", "char*", "void*", "size_t"},
            2, "Easy"
        },
        {
            103, "C Programming",
            "Which header file is required to use the 'bool' type in C99?",
            {"<stdbool.h>", "<boolean.h>", "<stdlib.h>", "<stdio.h>"},
            0, "Easy"
        },
        {
            104, "C Programming",
            "What is the result of dereferencing a NULL pointer in C?",
            {"Returns 0", "Undefined behaviour / Segmentation fault", "Throws an exception", "Allocates memory"},
            1, "Medium"
        },
        {
            105, "C Programming",
            "Which standard function is used to free heap-allocated memory in C?",
            {"delete()", "destroy()", "free()", "dealloc()"},
            2, "Easy"
        },
        {
            106, "C Programming",
            "What does the 'sizeof' operator evaluate to in C?",
            {"size_t", "int", "long long", "unsigned short"},
            0, "Medium"
        }
    };

    /* Data Structures Questions */
    Question ds_q[] = {
        {
            201, "Data Structures",
            "Which data structure follows the LIFO (Last In First Out) principle?",
            {"Queue", "Stack", "Binary Tree", "Heap"},
            1, "Easy"
        },
        {
            202, "Data Structures",
            "What is the average time complexity of Quicksort?",
            {"O(n)", "O(n^2)", "O(n log n)", "O(log n)"},
            2, "Medium"
        },
        {
            203, "Data Structures",
            "What is the time complexity to insert a new node at the head of a singly linked list?",
            {"O(1)", "O(n)", "O(log n)", "O(n log n)"},
            0, "Easy"
        },
        {
            204, "Data Structures",
            "Which data structure operates strictly on a FIFO (First In First Out) basis?",
            {"Stack", "Priority Queue", "Queue", "Binary Search Tree"},
            2, "Easy"
        },
        {
            205, "Data Structures",
            "What is the maximum number of children any node can possess in a binary tree?",
            {"1", "2", "3", "Unlimited"},
            1, "Easy"
        },
        {
            206, "Data Structures",
            "Which searching algorithm requires the input array to be pre-sorted?",
            {"Linear Search", "Binary Search", "Hash Lookup", "Breadth First Search"},
            1, "Medium"
        }
    };

    /* Computer Fundamentals Questions */
    Question cf_q[] = {
        {
            301, "Computer Fundamentals",
            "How many bits comprise one standard byte in modern computing?",
            {"4 bits", "8 bits", "16 bits", "32 bits"},
            1, "Easy"
        },
        {
            302, "Computer Fundamentals",
            "Which hardware component is primarily known as the brain of the computer?",
            {"RAM", "GPU", "CPU", "SSD"},
            2, "Easy"
        },
        {
            303, "Computer Fundamentals",
            "Which memory type is volatile and loses its data when powered off?",
            {"ROM", "RAM", "Hard Disk Drive", "Flash EEPROM"},
            1, "Easy"
        },
        {
            304, "Computer Fundamentals",
            "What is the primary role of the Operating System kernel?",
            {"Web browsing", "Managing hardware and system resources", "Compiling code", "Rendering 3D graphics"},
            1, "Medium"
        },
        {
            305, "Computer Fundamentals",
            "What does the acronym ALU stand for in processor architecture?",
            {"Arithmetic Logic Unit", "Advanced Linking Utility", "Application Layer Unit", "Array Line Unit"},
            0, "Easy"
        },
        {
            306, "Computer Fundamentals",
            "Which base numbering system utilizes digits 0-9 and letters A-F?",
            {"Binary (Base 2)", "Octal (Base 8)", "Decimal (Base 10)", "Hexadecimal (Base 16)"},
            3, "Easy"
        }
    };

    for (size_t i = 0; i < sizeof(c_q)/sizeof(c_q[0]); i++) {
        bank->questions[bank->count++] = c_q[i];
    }
    for (size_t i = 0; i < sizeof(ds_q)/sizeof(ds_q[0]); i++) {
        bank->questions[bank->count++] = ds_q[i];
    }
    for (size_t i = 0; i < sizeof(cf_q)/sizeof(cf_q[0]); i++) {
        bank->questions[bank->count++] = cf_q[i];
    }
}

bool question_bank_save_to_file(const QuestionBank *bank, const char *filepath) {
    if (!bank || !filepath) return false;
    FILE *fp = fopen(filepath, "w");
    if (!fp) return false;

    fprintf(fp, "%d\n", bank->count);
    for (int i = 0; i < bank->count; i++) {
        const Question *q = &bank->questions[i];
        fprintf(fp, "%d\n%s\n%s\n%s\n%s\n%s\n%s\n%d\n%s\n",
            q->id,
            q->topic,
            q->text,
            q->options[0],
            q->options[1],
            q->options[2],
            q->options[3],
            q->correct_index,
            q->difficulty
        );
    }

    fclose(fp);
    return true;
}

bool question_bank_load_from_file(QuestionBank *bank, const char *filepath) {
    if (!bank || !filepath) return false;
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        question_bank_load_defaults(bank);
        question_bank_save_to_file(bank, filepath);
        return true;
    }

    char line[512];
    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        question_bank_load_defaults(bank);
        question_bank_save_to_file(bank, filepath);
        return true;
    }

    int total = atoi(line);
    if (total <= 0 || total > MAX_BANK_QUESTIONS) {
        fclose(fp);
        question_bank_load_defaults(bank);
        question_bank_save_to_file(bank, filepath);
        return true;
    }

    bank->count = 0;
    for (int i = 0; i < total; i++) {
        Question q;
        memset(&q, 0, sizeof(Question));

        if (fgets(line, sizeof(line), fp) == NULL) break;
        q.id = atoi(line);

        if (fgets(line, sizeof(line), fp) == NULL) break;
        line[strcspn(line, "\r\n")] = '\0';
        strncpy(q.topic, line, sizeof(q.topic) - 1);

        if (fgets(line, sizeof(line), fp) == NULL) break;
        line[strcspn(line, "\r\n")] = '\0';
        strncpy(q.text, line, sizeof(q.text) - 1);

        for (int opt = 0; opt < 4; opt++) {
            if (fgets(line, sizeof(line), fp) == NULL) break;
            line[strcspn(line, "\r\n")] = '\0';
            strncpy(q.options[opt], line, sizeof(q.options[opt]) - 1);
        }

        if (fgets(line, sizeof(line), fp) == NULL) break;
        q.correct_index = atoi(line);
        if (q.correct_index < 0 || q.correct_index > 3) q.correct_index = 0;

        if (fgets(line, sizeof(line), fp) == NULL) break;
        line[strcspn(line, "\r\n")] = '\0';
        strncpy(q.difficulty, line, sizeof(q.difficulty) - 1);

        bank->questions[bank->count++] = q;
    }

    fclose(fp);
    if (bank->count == 0) {
        question_bank_load_defaults(bank);
        question_bank_save_to_file(bank, filepath);
    }
    return true;
}

int question_bank_get_by_topic(const QuestionBank *bank, const char *topic, Question *out_questions, int max_out) {
    if (!bank || !topic || !out_questions || max_out <= 0) return 0;
    int matched = 0;
    for (int i = 0; i < bank->count && matched < max_out; i++) {
        if (strcmp(bank->questions[i].topic, topic) == 0) {
            out_questions[matched++] = bank->questions[i];
        }
    }
    return matched;
}

void quiz_session_init(QuizSession *session) {
    if (!session) return;
    memset(session, 0, sizeof(QuizSession));
    queue_init(&session->question_queue);
    stack_init(&session->undo_stack);
    for (int i = 0; i < MAX_QUIZ_QUESTIONS; i++) {
        session->user_answers[i] = -1;
    }
    session->current_index = 0;
    session->total_questions = 0;
    session->is_submitted = false;
    session->is_active = false;
    session->time_remaining = 0.0f;
    session->total_duration = 0.0f;
}

bool quiz_session_start(QuizSession *session, const QuestionBank *bank, const char *topic, float time_limit_seconds) {
    if (!session || !bank || !topic) return false;
    quiz_session_cleanup(session);
    quiz_session_init(session);

    strncpy(session->topic, topic, sizeof(session->topic) - 1);
    session->total_questions = question_bank_get_by_topic(bank, topic, session->active_questions, MAX_QUIZ_QUESTIONS);
    if (session->total_questions == 0) {
        return false;
    }

    /* Initialize question sequence via Queue data structure (Requirement 6, 11.C) */
    for (int i = 0; i < session->total_questions; i++) {
        queue_enqueue(&session->question_queue, i);
        session->user_answers[i] = -1;
    }

    session->current_index = 0;
    session->is_active = true;
    session->is_submitted = false;
    session->total_duration = time_limit_seconds;
    session->time_remaining = time_limit_seconds;

    return true;
}

bool quiz_session_select_answer(QuizSession *session, int option_index) {
    if (!session || !session->is_active || session->is_submitted) return false;
    if (session->current_index < 0 || session->current_index >= session->total_questions) return false;
    if (option_index < 0 || option_index >= 4) return false;

    int prev = session->user_answers[session->current_index];
    session->user_answers[session->current_index] = option_index;

    /* Push action onto Stack for Undo support (Requirement 11.E) */
    AnswerAction action;
    action.question_index = session->current_index;
    action.previous_option = prev;
    action.new_option = option_index;
    stack_push(&session->undo_stack, action);

    return true;
}

bool quiz_session_undo_answer(QuizSession *session) {
    if (!session || !session->is_active || session->is_submitted) return false;
    if (stack_is_empty(&session->undo_stack)) return false;

    AnswerAction action;
    if (stack_pop(&session->undo_stack, &action)) {
        if (action.question_index >= 0 && action.question_index < session->total_questions) {
            session->user_answers[action.question_index] = action.previous_option;
            session->current_index = action.question_index;
            return true;
        }
    }
    return false;
}

bool quiz_session_next(QuizSession *session) {
    if (!session || !session->is_active) return false;
    if (session->current_index + 1 < session->total_questions) {
        session->current_index++;
        return true;
    }
    return false;
}

bool quiz_session_prev(QuizSession *session) {
    if (!session || !session->is_active) return false;
    if (session->current_index - 1 >= 0) {
        session->current_index--;
        return true;
    }
    return false;
}

bool quiz_session_jump_to(QuizSession *session, int index) {
    if (!session || !session->is_active) return false;
    if (index >= 0 && index < session->total_questions) {
        session->current_index = index;
        return true;
    }
    return false;
}

void quiz_session_submit(QuizSession *session, QuizAttempt *out_attempt, const char *student_id) {
    if (!session || session->is_submitted || !session->is_active) return;

    session->is_submitted = true;
    session->is_active = false;

    session->correct_count = 0;
    session->incorrect_count = 0;
    session->unanswered_count = 0;

    for (int i = 0; i < session->total_questions; i++) {
        int user_choice = session->user_answers[i];
        int correct_choice = session->active_questions[i].correct_index;

        if (user_choice == -1) {
            session->unanswered_count++;
        } else if (user_choice == correct_choice) {
            session->correct_count++;
        } else {
            session->incorrect_count++;
        }
    }

    if (session->total_questions > 0) {
        session->score_percentage = (session->correct_count * 100) / session->total_questions;
    } else {
        session->score_percentage = 0;
    }

    /* Assign performance category (Requirement 7) */
    if (session->score_percentage < 50) {
        strncpy(session->performance_category, "Needs Improvement", sizeof(session->performance_category) - 1);
    } else if (session->score_percentage < 80) {
        strncpy(session->performance_category, "Developing", sizeof(session->performance_category) - 1);
    } else {
        strncpy(session->performance_category, "Proficient", sizeof(session->performance_category) - 1);
    }

    if (out_attempt) {
        memset(out_attempt, 0, sizeof(QuizAttempt));
        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        snprintf(out_attempt->attempt_id, sizeof(out_attempt->attempt_id), "ATT-%ld", (long)now);
        strncpy(out_attempt->student_id, student_id ? student_id : "UNKNOWN", sizeof(out_attempt->student_id) - 1);
        strncpy(out_attempt->topic_name, session->topic, sizeof(out_attempt->topic_name) - 1);
        out_attempt->total_questions = session->total_questions;
        out_attempt->correct_answers = session->correct_count;
        out_attempt->incorrect_answers = session->incorrect_count;
        out_attempt->unanswered = session->unanswered_count;
        out_attempt->score_percentage = session->score_percentage;

        if (t) {
            strftime(out_attempt->timestamp, sizeof(out_attempt->timestamp), "%Y-%m-%d %H:%M", t);
        } else {
            snprintf(out_attempt->timestamp, sizeof(out_attempt->timestamp), "2026-10-13 12:00");
        }

        for (int i = 0; i < session->total_questions && i < MAX_QUIZ_QUESTIONS; i++) {
            out_attempt->question_ids[i] = session->active_questions[i].id;
            out_attempt->user_answers[i] = session->user_answers[i];
            out_attempt->correct_answers_key[i] = session->active_questions[i].correct_index;
        }
    }
}

void quiz_session_cleanup(QuizSession *session) {
    if (!session) return;
    queue_free(&session->question_queue);
    stack_free(&session->undo_stack);
}
