#ifndef DELETE_H
#define DELETE_H

#include "btree_mgr.h"
#include "buffer_mgr.h"
#include "structs.h"

Node *join_nodes(BTreeManager *tree_manager, Node *n, Node *neighbor, int neighbor_index, Value *k_prime);

Node *remove_entry(BTreeManager *tree_manager, Node *n, Value *key, Node *ptr);

Node *delete_entry(BTreeManager *tree_manager, Node *n, Value *key, void *ptr);
Node *delete_node(BTreeManager *tree_manager, Value *key);

Node *update_root(Node *root);
Node *update_nodes(Node *root, Node *n, Node *neighbor, int neighbor_index, int k_prime_index, Value *k_prime);

int get_index(Node *n);

#endif // DELETE_H