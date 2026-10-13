#include "student.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void trim_whitespace(char *str) {
    if (!str) return;
    char *end;
    /* Trim leading space */
    while (isspace((unsigned char)*str)) str++;
    /* Trim trailing space */
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
}

void student_init(Student *student) {
    if (!student) return;
    memset(student, 0, sizeof(Student));
    student->is_registered = false;
}

bool student_validate(const char *name, const char *student_id, char *out_error, size_t max_err_len) {
    if (out_error && max_err_len > 0) out_error[0] = '\0';

    if (!name || strlen(name) == 0) {
        if (out_error) snprintf(out_error, max_err_len, "Please enter your full name.");
        return false;
    }

    /* Check if name is only spaces */
    bool non_space_name = false;
    for (size_t i = 0; i < strlen(name); i++) {
        if (!isspace((unsigned char)name[i])) {
            non_space_name = true;
            break;
        }
    }
    if (!non_space_name) {
        if (out_error) snprintf(out_error, max_err_len, "Name cannot consist solely of whitespace.");
        return false;
    }

    if (!student_id || strlen(student_id) == 0) {
        if (out_error) snprintf(out_error, max_err_len, "Please enter your Student ID.");
        return false;
    }

    bool non_space_id = false;
    for (size_t i = 0; i < strlen(student_id); i++) {
        if (!isspace((unsigned char)student_id[i])) {
            non_space_id = true;
            break;
        }
    }
    if (!non_space_id) {
        if (out_error) snprintf(out_error, max_err_len, "Student ID cannot be empty or whitespace.");
        return false;
    }

    if (strlen(name) >= MAX_STUDENT_NAME_LEN - 1) {
        if (out_error) snprintf(out_error, max_err_len, "Name exceeds maximum length (%d characters).", MAX_STUDENT_NAME_LEN - 2);
        return false;
    }

    if (strlen(student_id) >= MAX_STUDENT_ID_LEN - 1) {
        if (out_error) snprintf(out_error, max_err_len, "Student ID exceeds maximum length (%d characters).", MAX_STUDENT_ID_LEN - 2);
        return false;
    }

    return true;
}

bool student_save_profile(const char *filepath, const Student *student) {
    if (!filepath || !student || !student->is_registered) return false;

    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        return false;
    }

    /* Simple, clean line-based format: NAME\nSTUDENT_ID\n */
    fprintf(fp, "%s\n%s\n", student->name, student->student_id);
    fclose(fp);
    return true;
}

bool student_load_profile(const char *filepath, Student *student) {
    if (!filepath || !student) return false;

    student_init(student);
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        return false;
    }

    char name_buf[MAX_STUDENT_NAME_LEN];
    char id_buf[MAX_STUDENT_ID_LEN];

    if (fgets(name_buf, sizeof(name_buf), fp) == NULL) {
        fclose(fp);
        return false;
    }
    if (fgets(id_buf, sizeof(id_buf), fp) == NULL) {
        fclose(fp);
        return false;
    }
    fclose(fp);

    /* Strip newline characters */
    name_buf[strcspn(name_buf, "\r\n")] = '\0';
    id_buf[strcspn(id_buf, "\r\n")] = '\0';

    trim_whitespace(name_buf);
    trim_whitespace(id_buf);

    if (strlen(name_buf) > 0 && strlen(id_buf) > 0) {
        strncpy(student->name, name_buf, sizeof(student->name) - 1);
        student->name[sizeof(student->name) - 1] = '\0';

        strncpy(student->student_id, id_buf, sizeof(student->student_id) - 1);
        student->student_id[sizeof(student->student_id) - 1] = '\0';

        student->is_registered = true;
        return true;
    }

    return false;
}
