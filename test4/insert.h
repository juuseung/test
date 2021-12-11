#ifndef INSERT_H
#define INSERT_H

#include "btree_mgr.h"
#include "buffer_mgr.h"
#include "structs.h"

NodeValue *new_record (RID *rid);

Node *split_to_leaf (Value *key, NodeValue *ptr, Node *leaf, BTreeInfo *tree_manager);
Node *split_to_node (Node *parent_node, Value *key, Node *r_node, BTreeInfo *tree_manager, int left_idx);

Node *empty_tree (Value *key, NodeValue *ptr, BTreeInfo *tree_manager);
Node *empty_node (BTreeInfo *tree_manager);
Node *empty_leaf (BTreeInfo *tree_manager);

Node *add_to_leaf (Value *key, NodeValue *ptr, Node *leaf, BTreeInfo *tree_manager);
Node *add_to_node (Node *parent_node, int left_idx, Value *key, Node *r_node, BTreeInfo *tree_manager);
Node *add_to_parent ( Node *l_node, Value *key, Node *r_node, BTreeInfo *tree_manager);
Node *add_to_empty_root (Node *l_node, Value *key, Node *r_node, BTreeInfo *tree_manager);

int get_left (Node *l_node, Node *parent_node);

#endif // INSERT_H