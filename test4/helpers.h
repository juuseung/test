#ifndef HELPERS_H
#define HELPERS_H

#include "buffer_mgr.h"
#include "btree_mgr.h"
#include "structs.h"

bool is_less (Value *k1, Value *k2);
bool is_equal (Value *k1, Value *k2);
bool is_greater (Value *k1, Value *k2);

int path (Node *child, Node *root);

Node *dequeue (BTreeInfo *tree_manager);
void enqueue (Node *new_node, BTreeInfo *tree_manager);

Node *leaf_search (Value *key, Node *root);
NodeValue *record_search (Value *key, Node *root);

#endif // HELPERS_H