//
// Created by hugo1 on 21.03.2019.
//

#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "logger.hpp"

struct SNode {
    void *data;
    struct SNode *next;
    bool sentinel;
};

struct SNode *SNode_init() {
    struct SNode *node = (struct SNode *) malloc(sizeof(struct SNode));
    node->sentinel = false;
    node->next = node;
    node->data = nullptr;
    return node;
}

struct SNode *SNode_sent_init() {
    struct SNode *node = SNode_init();
    node->sentinel = true;
    return node;
}

bool isNodeEmpty(struct SNode *head) {
    if (head->sentinel && head->next->sentinel)
        return true;
    else
        return false;
}

void *popRear(struct SNode **headRef) { // Pop queue style, aka. FIFO
    struct SNode *head = *headRef;
    while (!head->next->next->sentinel)
        head = head->next;

    if (head->next->sentinel) {
        logger.error("Queue is empty.\n");
        return nullptr;
    } else {
        struct SNode *toBeRemoved = head->next;
        head->next = toBeRemoved->next;
        void *result = toBeRemoved->data;
        free(toBeRemoved);
        return result;
    }
}

void *popFront(struct SNode **headRef) { // Pop stack style, aka. FILO
    struct SNode *head = *headRef;
    while (!head->sentinel)
        head = head->next;

    if (head->next->sentinel) {
        logger.error("Stack is empty.\n");
        return nullptr;
    } else {
        struct SNode *toBeRemoved = head->next;
        head->next = toBeRemoved->next;
        void *result = toBeRemoved->data;
        free(toBeRemoved);
        return result;
    }
}

void *readFront(struct SNode **headRef) {
    struct SNode *head = *headRef;
    while (!head->sentinel)
        head = head->next;

    if (head->next->sentinel) {
        logger.error("Stack is empty.\n");
        return nullptr;
    } else {
        return head->next->data;
    }
}

void push(struct SNode **headRef, void *data) {
    struct SNode *head = *headRef;
    while (!head->sentinel)
        head = head->next;

    struct SNode *newNode = SNode_init();
    newNode->data = data;
    newNode->next = head->next;
    head->next = newNode; // New node added just behind the sentinel
}
