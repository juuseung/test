#ifndef HELPERS_H
#define HELPERS_H

#include "btree_mgr.h"
#include "buffer_mgr.h"
#include "structs.h"

bool is_equal(Value *k1, Value *k2);
bool is_less(Value *k1, Value *k2);
bool is_greater(Value *k1, Value *k2);

void enqueue(BTreeManager *tree_manager, Node *new_node);
Node *dequeue(BTreeManager *tree_manager);

NodeData *record_search(Node *root, Value *key);
Node *leaf_search(Node *root, Value *key);

int path(Node *root, Node *child);

#endif // HELPERS_H