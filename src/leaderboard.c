#include "leaderboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

void leaderboard_init(Leaderboard *lb) {
    if (!lb) return;
    memset(lb, 0, sizeof(Leaderboard));
    lb->count = 0;
}

void leaderboard_seed_peers(Leaderboard *lb) {
    if (!lb) return;
    leaderboard_init(lb);

    /* Local cohort benchmark peers */
    LeaderboardEntry peers[] = {
        {0, "Sophia Martinez", "CS-2024-001", 92, 14, false},
        {0, "Ethan Reynolds",  "CS-2024-012", 87, 11, false},
        {0, "Aarav Sharma",    "CS-2024-034", 81, 9,  false},
        {0, "Maya Patel",      "CS-2024-055", 76, 7,  false},
        {0, "Liam Chen",       "CS-2024-082", 68, 5,  false},
        {0, "Chloe Dubois",    "CS-2024-099", 54, 4,  false}
    };

    int num_peers = sizeof(peers) / sizeof(peers[0]);
    for (int i = 0; i < num_peers && lb->count < MAX_LEADERBOARD_ENTRIES; i++) {
        lb->entries[lb->count++] = peers[i];
    }
}

bool leaderboard_load_or_seed(Leaderboard *lb, const char *filepath) {
    if (!lb) return false;
    leaderboard_init(lb);

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        leaderboard_seed_peers(lb);
        leaderboard_save_to_file(lb, filepath);
        return true;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL && lb->count < MAX_LEADERBOARD_ENTRIES) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        char *token = strtok(line, "|");
        if (!token) continue;
        char name[MAX_STUDENT_NAME_LEN];
        strncpy(name, token, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';

        token = strtok(NULL, "|");
        if (!token) continue;
        char sid[MAX_STUDENT_ID_LEN];
        strncpy(sid, token, sizeof(sid) - 1);
        sid[sizeof(sid) - 1] = '\0';

        token = strtok(NULL, "|");
        if (!token) continue;
        int avg = atoi(token);

        token = strtok(NULL, "|");
        if (!token) continue;
        int completed = atoi(token);

        LeaderboardEntry *e = &lb->entries[lb->count++];
        e->rank = 0;
        strncpy(e->name, name, sizeof(e->name) - 1);
        strncpy(e->student_id, sid, sizeof(e->student_id) - 1);
        e->average_percentage = avg;
        e->quizzes_completed = completed;
        e->is_current_student = false;
    }

    fclose(fp);
    if (lb->count == 0) {
        leaderboard_seed_peers(lb);
        leaderboard_save_to_file(lb, filepath);
    }
    return true;
}

bool leaderboard_save_to_file(const Leaderboard *lb, const char *filepath) {
    if (!lb || !filepath) return false;
    FILE *fp = fopen(filepath, "w");
    if (!fp) return false;

    for (int i = 0; i < lb->count; i++) {
        const LeaderboardEntry *e = &lb->entries[i];
        fprintf(fp, "%s|%s|%d|%d\n", e->name, e->student_id, e->average_percentage, e->quizzes_completed);
    }

    fclose(fp);

#if defined(__EMSCRIPTEN__)
    EM_ASM(if (typeof FS !== 'undefined' && FS.syncfs) FS.syncfs(false, function(){}););
#endif

    return true;
}

void leaderboard_update_current_student(Leaderboard *lb, const Student *current_student, const OverallProgress *progress) {
    if (!lb || !current_student || !current_student->is_registered) return;

    /* Check if student already exists in entries */
    int existing_idx = -1;
    for (int i = 0; i < lb->count; i++) {
        if (strcmp(lb->entries[i].student_id, current_student->student_id) == 0) {
            existing_idx = i;
            break;
        }
    }

    if (existing_idx != -1) {
        LeaderboardEntry *e = &lb->entries[existing_idx];
        strncpy(e->name, current_student->name, sizeof(e->name) - 1);
        e->average_percentage = progress ? progress->overall_average_percentage : 0;
        e->quizzes_completed = progress ? progress->total_quizzes : 0;
        e->is_current_student = true;
    } else {
        if (lb->count < MAX_LEADERBOARD_ENTRIES) {
            LeaderboardEntry *e = &lb->entries[lb->count++];
            strncpy(e->name, current_student->name, sizeof(e->name) - 1);
            strncpy(e->student_id, current_student->student_id, sizeof(e->student_id) - 1);
            e->average_percentage = progress ? progress->overall_average_percentage : 0;
            e->quizzes_completed = progress ? progress->total_quizzes : 0;
            e->is_current_student = true;
        }
    }

    leaderboard_sort(lb);
}

/* ========================================================================= */
/* QUICKSORT IMPLEMENTATION (Requirement 9 & 11.G)                           */
/* Sort keys:                                                                */
/*   1. Primary: average_percentage (Descending)                             */
/*   2. Secondary tie-breaker: quizzes_completed (Descending)                */
/*   3. Tertiary: alphabetical by name (Ascending)                           */
/* ========================================================================= */

static int compare_entries(const LeaderboardEntry *a, const LeaderboardEntry *b) {
    if (a->average_percentage != b->average_percentage) {
        return (b->average_percentage - a->average_percentage); /* higher avg first */
    }
    if (a->quizzes_completed != b->quizzes_completed) {
        return (b->quizzes_completed - a->quizzes_completed); /* more completed first */
    }
    return strcmp(a->name, b->name);
}

static void swap_entries(LeaderboardEntry *a, LeaderboardEntry *b) {
    LeaderboardEntry tmp = *a;
    *a = *b;
    *b = tmp;
}

static int partition(LeaderboardEntry arr[], int low, int high) {
    LeaderboardEntry pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j < high; j++) {
        if (compare_entries(&arr[j], &pivot) <= 0) {
            i++;
            swap_entries(&arr[i], &arr[j]);
        }
    }
    swap_entries(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void leaderboard_quicksort(LeaderboardEntry entries[], int low, int high) {
    if (low < high) {
        int pi = partition(entries, low, high);
        leaderboard_quicksort(entries, low, pi - 1);
        leaderboard_quicksort(entries, pi + 1, high);
    }
}

void leaderboard_sort(Leaderboard *lb) {
    if (!lb || lb->count <= 1) return;
    leaderboard_quicksort(lb->entries, 0, lb->count - 1);

    /* Assign rank numbers 1..count */
    for (int i = 0; i < lb->count; i++) {
        lb->entries[i].rank = i + 1;
    }
}

/* ========================================================================= */
/* SEARCHING ALGORITHM (Requirement 9 & 11.F)                                */
/* Linear / ID search for looking up a student record by their student ID    */
/* ========================================================================= */

int leaderboard_search_by_id(const Leaderboard *lb, const char *student_id) {
    if (!lb || !student_id) return -1;
    for (int i = 0; i < lb->count; i++) {
        if (strcmp(lb->entries[i].student_id, student_id) == 0) {
            return i;
        }
    }
    return -1;
}
