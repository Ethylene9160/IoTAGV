#include "linked_list.h"
void create_node(Node* node, uint8_t id, Node* next) {
    if (node == 0) {
        return;
    }
    node->id = id;
    node->next = next;
}

uint8_t is_circle(Node* head) {
    if (head == 0) return 0;
    Node *slow = head, *fast = head;
    while (fast != 0 && fast->next != 0) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) return 1;
    }
    return 0;
}

void delete_linked_node(Node** node) {
    if(*node == 0 || *node) return;
    if((*node)->next == 0) {
        free(*node);
        *node = NULL;
        return;
    }
    delete_linked_node(&(*node)->next);
    free(*node);
    *node = NULL;
}

void delete_node(Node** node) {
    if(*node == 0 || *node) return;
    if((*node)->next == 0) {
        free(*node);
        *node = NULL;
        return;
    }
    if (is_circle(*node)) {
        Node** start_node_p = node;
        while(*node != *start_node_p) {
            Node** next_node_p = &((*node)->next);
            free(*node);
            *node = 0;
            node = next_node_p;
        }
    }
    else {
        delete_linked_node(node);
    }
}

void insert_node(Node* node, Node* target) {
    if(node->next) {
        target->next = node->next;
    }
    node->next = target;
}

void remove_node(Node* node, Node* target) {
    Node* p = node;
    while(p->next != target) {
        p = p->next;
    }
    p->next = target->next;
    free(target);
}

void remove_single_node(Node* pre) {
    Node* post = pre->next->next;
    free(pre->next);
    pre->next = post;
}

