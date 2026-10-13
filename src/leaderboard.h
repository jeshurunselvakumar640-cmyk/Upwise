#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <stdbool.h>
#include "student.h"
#include "progress.h"

#define MAX_LEADERBOARD_ENTRIES 32

typedef struct {
    int rank;
    char name[MAX_STUDENT_NAME_LEN];
    char student_id[MAX_STUDENT_ID_LEN];
    int average_percentage;
    int quizzes_completed;
    bool is_current_student;
} LeaderboardEntry;

typedef struct {
    LeaderboardEntry entries[MAX_LEADERBOARD_ENTRIES];
    int count;
} Leaderboard;

void leaderboard_init(Leaderboard *lb);
void leaderboard_seed_peers(Leaderboard *lb);
bool leaderboard_load_or_seed(Leaderboard *lb, const char *filepath);
bool leaderboard_save_to_file(const Leaderboard *lb, const char *filepath);
void leaderboard_update_current_student(Leaderboard *lb, const Student *current_student, const OverallProgress *progress);

/* Sorting Algorithms (Requirement 9 & 11.G) */
void leaderboard_quicksort(LeaderboardEntry entries[], int low, int high);
void leaderboard_sort(Leaderboard *lb);

/* Searching Algorithms (Requirement 9 & 11.F) */
int leaderboard_search_by_id(const Leaderboard *lb, const char *student_id);

#endif /* LEADERBOARD_H */
