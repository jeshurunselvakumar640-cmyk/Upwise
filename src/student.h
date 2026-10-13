#ifndef STUDENT_H
#define STUDENT_H

#include <stdbool.h>
#include <stddef.h>

#define MAX_STUDENT_NAME_LEN 64
#define MAX_STUDENT_ID_LEN 32

typedef struct {
    char name[MAX_STUDENT_NAME_LEN];
    char student_id[MAX_STUDENT_ID_LEN];
    bool is_registered;
} Student;

void student_init(Student *student);
bool student_validate(const char *name, const char *student_id, char *out_error, size_t max_err_len);
bool student_save_profile(const char *filepath, const Student *student);
bool student_load_profile(const char *filepath, Student *student);

#endif /* STUDENT_H */
