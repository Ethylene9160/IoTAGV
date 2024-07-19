#ifndef LINKED_LIST_H
#define LINKED_LIST_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct LinkedNode{
    uint8_t id;
    struct LinkedNode *next;
}Node;

void create_node(Node* node, uint8_t id, Node* next);

uint8_t is_circle(Node* head);

void delete_linked_node(Node** node);

void delete_node(Node** node);

void insert_node(Node* node, Node* target);

void remove_node(Node* node, Node* target);

void remove_single_node(Node* pre);
#endif //LINKED_LIST_H
