#ifndef DELETE_H
#define DELETE_H

#include "btree_mgr.h"
#include "buffer_mgr.h"
#include "structs.h"

Node *join_nodes (Node *n, Node *adj, int adj_idx, BTreeInfo *tree_manager, Value *kp);

Node *remove_entry (Node *n, Node *ptr, Value *key, BTreeInfo *tree_manager);

Node *delete_entry (Node *n, Value *key, void *ptr, BTreeInfo *tree_manager);
Node *delete_node (Value *key, BTreeInfo *tree_manager);

Node *update_root (Node *root);
Node *update_nodes (Node *root, Node *n, Node *adj, Value *kp, int adj_idx, int kpidx);

int get_index (Node *n);

#endif // DELETE_H