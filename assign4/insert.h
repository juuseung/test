#ifndef INSERT_H
#define INSERT_H

#include "btree_mgr.h"
#include "buffer_mgr.h"
#include "structs.h"

NodeData *new_record(RID *rid);

Node *split_to_leaf(Value *key, NodeData *ptr, Node *leaf, BTreeManager *tree_manager);
Node *split_to_node(BTreeManager *tree_manager, Node *parent, int left_index, Value *key, Node *right);

Node *empty_tree(BTreeManager *tree_manager, Value *key, NodeData *ptr);
Node *empty_node(BTreeManager *tree_manager);
Node *empty_leaf(BTreeManager *tree_manager);

Node *add_to_leaf(Value *key, NodeData *ptr, Node *leaf, BTreeManager *tree_manager);
Node *add_to_node(BTreeManager *tree_manager, Node *parent, int left_index, Value *key, Node *right);
Node *add_to_parent(BTreeManager *tree_manager, Node *left, Value *key, Node *right);
Node *add_to_empty_root(BTreeManager *tree_manager, Node *left, Value *key, Node *right);

int get_left(Node *parent, Node *left);

#endif // INSERT_H