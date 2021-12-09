#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "helpers.h"
#include "delete.h"
#include "dt.h"
#include "string.h"

int get_index(Node *n) {
	for (int i = 0; i <= n->parent->num_keys; i++) if (n->parent->ptrs[i] == n) return i - 1;
	printf("Node:  %#lx\n", (unsigned long) n);
	exit(RC_SEARCH_FAILED);
}

Node *remove_entry(BTreeManager *tree_manager, Node *n, Value *key, Node *ptr) {

	int i, num_ptrs;
	int bTreeOrder = tree_manager->order;

	i = 0;
	while (!is_equal(n->keys[i], key)) i++;
	for (++i; i < n->num_keys; i++) n->keys[i - 1] = n->keys[i];

	num_ptrs = n->leaf_flag ? n->num_keys : n->num_keys + 1;

	i = 0;
	while (n->ptrs[i] != ptr) i++;
	for (++i; i < num_ptrs; i++) n->ptrs[i - 1] = n->ptrs[i];

	n->num_keys--, tree_manager->num_entries--;

	if (n->leaf_flag) for (i = n->num_keys; i < bTreeOrder - 1; i++) n->ptrs[i] = NULL;
	else for (i = n->num_keys + 1; i < bTreeOrder; i++) n->ptrs[i] = NULL;

	return n;
}

Node *update_root(Node *root) {

	Node *new_root;

	if (root->num_keys > 0) return root;

	if (root->leaf_flag) new_root = NULL;
    else {
		new_root = root->ptrs[0];
		new_root->parent = NULL;
	}

	free(root->keys), free(root->ptrs), free(root);

	return new_root;
}

Node *join_nodes(BTreeManager *tree_manager, Node *n, Node *neighbor, int neighbor_index, Value *k_prime) {

	int i, j, neighbor_insertion_index, n_end;
	Node *tmp;
	int bTreeOrder = tree_manager->order;

	if (neighbor_index == -1) {
		tmp = n;
		n = neighbor;
		neighbor = tmp;
	}

	neighbor_insertion_index = neighbor->num_keys;

	if (!n->leaf_flag) {

		neighbor->keys[neighbor_insertion_index] = k_prime;
		neighbor->num_keys++;

		n_end = n->num_keys;

		for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++) {
			neighbor->keys[i] = n->keys[j], neighbor->ptrs[i] = n->ptrs[j];
			neighbor->num_keys++, n->num_keys--;
		}

		neighbor->ptrs[i] = n->ptrs[j];

		for (i = 0; i < neighbor->num_keys + 1; i++) {
			tmp = (Node *) neighbor->ptrs[i];
			tmp->parent = neighbor;
		}

	} else {

		for (i = neighbor_insertion_index, j = 0; j < n->num_keys; i++, j++) {
			neighbor->keys[i] = n->keys[j], neighbor->ptrs[i] = n->ptrs[j];
			neighbor->num_keys++;
		}

		neighbor->ptrs[bTreeOrder - 1] = n->ptrs[bTreeOrder - 1];
	}

	tree_manager->root = delete_entry(tree_manager, n->parent, k_prime, n);

	free(n->keys), free(n->ptrs), free(n);

	return tree_manager->root;
}

Node *delete_entry(BTreeManager *tree_manager, Node *n, Value *key, void *ptr) {

	int min_keys, neighbor_index, k_prime_index, capacity;
	Node *neighbor;
	Value * k_prime;
	int bTreeOrder = tree_manager->order;

	n = remove_entry(tree_manager, n, key, ptr);

	if (n == tree_manager->root) return update_root(tree_manager->root);

	if (n->leaf_flag) {
		if ((bTreeOrder - 1) % 2 == 0) min_keys = (bTreeOrder - 1) / 2;
		else min_keys = (bTreeOrder - 1) / 2 + 1;
	} else {
		if ((bTreeOrder) % 2 == 0) min_keys = (bTreeOrder) / 2;
		else min_keys = (bTreeOrder) / 2 + 1;
		min_keys--;
	}

	if (n->num_keys >= min_keys) return tree_manager->root;

	neighbor_index = get_index(n);
	k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
	k_prime = (Value *) n->parent->keys[k_prime_index];
	neighbor = (neighbor_index == -1) ? n->parent->ptrs[1] : n->parent->ptrs[neighbor_index];

	capacity = n->leaf_flag ? bTreeOrder : bTreeOrder - 1;

	if (neighbor->num_keys + n->num_keys < capacity) return join_nodes(tree_manager, n, neighbor, neighbor_index, (Value *) k_prime);
	return update_nodes(tree_manager->root, n, neighbor, neighbor_index, k_prime_index, k_prime);
}

Node *delete_node(BTreeManager *tree_manager, Value *key) {

	NodeData *record = record_search(tree_manager->root, key);
	Node *key_leaf = leaf_search(tree_manager->root, key);

	if (record != NULL && key_leaf != NULL) {
		tree_manager->root = delete_entry(tree_manager, key_leaf, key, record);
		free(record);
	}

	return tree_manager->root;
}

Node *update_nodes(Node *root, Node *n, Node *neighbor, int neighbor_index, int k_prime_index, Value *k_prime) {

	int i;
	Node *tmp;

	if (neighbor_index != -1) {

		if (!n->leaf_flag) n->ptrs[n->num_keys + 1] = n->ptrs[n->num_keys];
		for (i = n->num_keys; i > 0; i--) {
			n->keys[i] = n->keys[i - 1]; n->ptrs[i] = n->ptrs[i - 1];
		}

		if (!n->leaf_flag) {
			n->ptrs[0] = neighbor->ptrs[neighbor->num_keys];
			tmp = (Node *) n->ptrs[0];
			tmp->parent = n;
			neighbor->ptrs[neighbor->num_keys] = NULL;
			n->keys[0] = k_prime;
			n->parent->keys[k_prime_index] = neighbor->keys[neighbor->num_keys - 1];
		} else {
			n->ptrs[0] = neighbor->ptrs[neighbor->num_keys - 1];
			neighbor->ptrs[neighbor->num_keys - 1] = NULL;
			n->keys[0] = neighbor->keys[neighbor->num_keys - 1];
			n->parent->keys[k_prime_index] = n->keys[0];
		}

	} else {

		if (n->leaf_flag) {
			n->keys[n->num_keys] = neighbor->keys[0]; n->ptrs[n->num_keys] = neighbor->ptrs[0];
			n->parent->keys[k_prime_index] = neighbor->keys[1];
		} else {
			n->keys[n->num_keys] = k_prime;
			n->ptrs[n->num_keys + 1] = neighbor->ptrs[0];
			tmp = (Node *) n->ptrs[n->num_keys + 1];
			tmp->parent = n;
			n->parent->keys[k_prime_index] = neighbor->keys[0];
		}
		for (i = 0; i < neighbor->num_keys - 1; i++)
			neighbor->keys[i] = neighbor->keys[i + 1], neighbor->ptrs[i] = neighbor->ptrs[i + 1];
		if (!n->leaf_flag) neighbor->ptrs[i] = neighbor->ptrs[i + 1];
	}
	n->num_keys++, neighbor->num_keys--;
	return root;
}