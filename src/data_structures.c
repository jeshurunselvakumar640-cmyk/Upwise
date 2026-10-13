#include "data_structures.h"
#include <stdlib.h>
#include <string.h>

/* ========================================================================= */
/* 1. QUEUE IMPLEMENTATION                                                   */
/* ========================================================================= */

void queue_init(Queue *q) {
    if (!q) return;
    q->front = NULL;
    q->rear = NULL;
    q->count = 0;
}

bool queue_is_empty(const Queue *q) {
    return (!q || q->count == 0 || q->front == NULL);
}

bool queue_enqueue(Queue *q, int question_index) {
    if (!q) return false;
    QueueNode *newNode = (QueueNode *)malloc(sizeof(QueueNode));
    if (!newNode) return false;
    newNode->question_index = question_index;
    newNode->next = NULL;

    if (q->rear == NULL) {
        q->front = newNode;
        q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->count++;
    return true;
}

bool queue_dequeue(Queue *q, int *out_question_index) {
    if (!q || queue_is_empty(q)) return false;
    QueueNode *temp = q->front;
    if (out_question_index) {
        *out_question_index = temp->question_index;
    }
    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    q->count--;
    return true;
}

int queue_count(const Queue *q) {
    return q ? q->count : 0;
}

void queue_free(Queue *q) {
    if (!q) return;
    int dummy;
    while (queue_dequeue(q, &dummy));
    q->front = NULL;
    q->rear = NULL;
    q->count = 0;
}

/* ========================================================================= */
/* 2. STACK IMPLEMENTATION                                                   */
/* ========================================================================= */

void stack_init(Stack *s) {
    if (!s) return;
    s->top = NULL;
    s->count = 0;
}

bool stack_is_empty(const Stack *s) {
    return (!s || s->count == 0 || s->top == NULL);
}

bool stack_push(Stack *s, AnswerAction action) {
    if (!s) return false;
    StackNode *newNode = (StackNode *)malloc(sizeof(StackNode));
    if (!newNode) return false;
    newNode->action = action;
    newNode->next = s->top;
    s->top = newNode;
    s->count++;
    return true;
}

bool stack_pop(Stack *s, AnswerAction *out_action) {
    if (!s || stack_is_empty(s)) return false;
    StackNode *temp = s->top;
    if (out_action) {
        *out_action = temp->action;
    }
    s->top = s->top->next;
    free(temp);
    s->count--;
    return true;
}

bool stack_peek(const Stack *s, AnswerAction *out_action) {
    if (!s || stack_is_empty(s)) return false;
    if (out_action) {
        *out_action = s->top->action;
    }
    return true;
}

int stack_count(const Stack *s) {
    return s ? s->count : 0;
}

void stack_free(Stack *s) {
    if (!s) return;
    AnswerAction dummy;
    while (stack_pop(s, &dummy));
    s->top = NULL;
    s->count = 0;
}

/* ========================================================================= */
/* 3. LINKED LIST IMPLEMENTATION                                             */
/* ========================================================================= */

void attempt_list_init(AttemptList *list) {
    if (!list) return;
    list->head = NULL;
    list->count = 0;
}

bool attempt_list_add(AttemptList *list, const QuizAttempt *attempt) {
    if (!list || !attempt) return false;
    AttemptNode *newNode = (AttemptNode *)malloc(sizeof(AttemptNode));
    if (!newNode) return false;
    memcpy(&newNode->attempt, attempt, sizeof(QuizAttempt));
    newNode->next = NULL;

    /* Append to end to maintain chronological order */
    if (list->head == NULL) {
        list->head = newNode;
    } else {
        AttemptNode *curr = list->head;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = newNode;
    }
    list->count++;
    return true;
}

int attempt_list_count(const AttemptList *list) {
    return list ? list->count : 0;
}

const QuizAttempt* attempt_list_get_at(const AttemptList *list, int index) {
    if (!list || index < 0 || index >= list->count) return NULL;
    AttemptNode *curr = list->head;
    int i = 0;
    while (curr != NULL) {
        if (i == index) return &curr->attempt;
        curr = curr->next;
        i++;
    }
    return NULL;
}

const QuizAttempt* attempt_list_find_by_id(const AttemptList *list, const char *attempt_id) {
    if (!list || !attempt_id) return NULL;
    AttemptNode *curr = list->head;
    while (curr != NULL) {
        if (strcmp(curr->attempt.attempt_id, attempt_id) == 0) {
            return &curr->attempt;
        }
        curr = curr->next;
    }
    return NULL;
}

void attempt_list_free(AttemptList *list) {
    if (!list) return;
    AttemptNode *curr = list->head;
    while (curr != NULL) {
        AttemptNode *next = curr->next;
        free(curr);
        curr = next;
    }
    list->head = NULL;
    list->count = 0;
}
