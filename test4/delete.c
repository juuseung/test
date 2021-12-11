#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "helpers.h"
#include "delete.h"

Node *join_nodes (Node *n, Node *adj, int adj_idx, BTreeInfo *tree_manager, Value *kp) {
	Node *temp;
	int i, j;
	if (adj_idx == -1) {
		temp = n;
		n = adj;
		adj = temp;
	}
	int idx = adj->num_keys;
	if (!n->leaf_flag) {
		adj->keys[idx] = kp;
		adj->num_keys++;
		adj->ptrs[i] = n->ptrs[j];
		for (i = 0; i < adj->num_keys + 1; i++) {
			temp = (Node *) adj->ptrs[i];
			temp->parent_node = adj;
		}
		for (i = idx + 1, j = 0; j < n->num_keys; i++, j++) {
			adj->keys[i] = n->keys[j], adj->ptrs[i] = n->ptrs[j];
			adj->num_keys++, n->num_keys--;
		}
	} else {
		for (i = idx, j = 0; j < n->num_keys; i++, j++) {
			adj->ptrs[i] = n->ptrs[j], adj->keys[i] = n->keys[j];
			adj->num_keys++;
		}
		adj->ptrs[tree_manager->order - 1] = n->ptrs[tree_manager->order - 1];
	}
	tree_manager->root = delete_entry(n->parent_node, kp, n, tree_manager);
	free(n->ptrs = NULL), free(n->keys = NULL), free(n = NULL);
	return tree_manager->root;
}

Node *remove_entry (Node *n, Node *ptr, Value *key, BTreeInfo *tree_manager) {
	int i = 0;
	while (is_greater(n->keys[i], key) || is_less(n->keys[i], key)) i++;
	++i;
	while(i<n->num_keys){ n->keys[i-1]=n->keys[i]; i++; }
	int num_ptrs;
	if(n->leaf_flag) num_ptrs = n->num_keys;
	else num_ptrs = n->num_keys + 1;
	i = 0;
	while (n->ptrs[i] != ptr) i++;
	++i;
	while(i<num_ptrs){ n->ptrs[i-1]=n->ptrs[i]; i++; }
	tree_manager->num_entries--, n->num_keys--;
	if (n->leaf_flag) for (i = n->num_keys; i < tree_manager->order - 1; i++) n->ptrs[i] = NULL;
	else for (i = n->num_keys + 1; i < tree_manager->order; i++) n->ptrs[i] = NULL;
	return n;
}

Node *update_root (Node *root) {
	Node *new_root;
	if (root->num_keys > 0) return root;
	if (root->leaf_flag) new_root = NULL;
    else {
		new_root = root->ptrs[0];
		new_root->parent_node = NULL;
	}
	free(root->ptrs = NULL), free(root->keys = NULL), free(root = NULL);
	return new_root;
}

int get_index (Node *n) {
	int i=0;
	while(i<=n->parent_node->num_keys){
		if(n->parent_node->ptrs[i] == n) return i-1;
		i++;
	}
	exit(RC_SEARCH_FAILED);
}

Node *delete_entry (Node *n, Value *key, void *ptr, BTreeInfo *tree_manager) {
	int min_keys;
	Node *adj;
	Value * kp;
	n = remove_entry(n, ptr, key, tree_manager);
	if (n == tree_manager->root) return update_root(tree_manager->root);
	if (n->leaf_flag) {
		if ((tree_manager->order-1) % 2 == 0) min_keys = (tree_manager->order-1) / 2;
		else min_keys = (tree_manager->order-1) / 2 + 1;
	} else {
		if ((tree_manager->order) % 2 == 0) min_keys = (tree_manager->order) / 2;
		else min_keys = (tree_manager->order) / 2 + 1;
		min_keys--;
	}
	if (n->num_keys >= min_keys) return tree_manager->root;
	int adj_idx = get_index(n);
	int kpidx = adj_idx == -1 ? 0 : adj_idx;
	kp = (Value *) n->parent_node->keys[kpidx];
	adj = (adj_idx == -1) ? n->parent_node->ptrs[1] : n->parent_node->ptrs[adj_idx];
	if (adj->num_keys + n->num_keys < (n->leaf_flag ? tree_manager->order : tree_manager->order - 1)) return join_nodes(n, adj, adj_idx, tree_manager, (Value *) kp);
	return update_nodes(tree_manager->root, n, adj, kp, adj_idx, kpidx);
}

Node *update_nodes (Node *root, Node *n, Node *adj, Value *kp, int adj_idx, int kpidx) {
	Node *temp;
	int i;
	if (adj_idx < 0) {
		if (!n->leaf_flag) {
			n->ptrs[n->num_keys+1] = adj->ptrs[0], n->keys[n->num_keys] = kp;
			temp = (Node *) n->ptrs[n->num_keys+1];
			temp->parent_node = n;
			n->parent_node->keys[kpidx] = adj->keys[0];
		} else {
			n->keys[n->num_keys] = adj->keys[0]; n->ptrs[n->num_keys] = adj->ptrs[0];
			n->parent_node->keys[kpidx] = adj->keys[1];
		}
		for (i = 0; i < adj->num_keys-1; i++)
			adj->keys[i] = adj->keys[i+1], adj->ptrs[i] = adj->ptrs[i+1];
		if (!n->leaf_flag) adj->ptrs[i] = adj->ptrs[i+1];
	} else {
		if (!n->leaf_flag) n->ptrs[n->num_keys + 1] = n->ptrs[n->num_keys];
		for (i = n->num_keys; i > 0; i--) {
			n->keys[i] = n->keys[i - 1]; n->ptrs[i] = n->ptrs[i-1];
		}
		if (n->leaf_flag) {
			n->keys[0] = adj->keys[adj->num_keys-1], n->parent_node->keys[kpidx] = n->keys[0];
			n->ptrs[0] = adj->ptrs[adj->num_keys-1], adj->ptrs[adj->num_keys-1] = NULL;
		} else {
			temp = (Node *) n->ptrs[0];
			n->ptrs[0] = adj->ptrs[adj->num_keys];
			n->keys[0] = kp, n->parent_node->keys[kpidx] = adj->keys[adj->num_keys-1];
			temp->parent_node = n;
			adj->ptrs[adj->num_keys] = NULL;
		}
	}
	n->num_keys++, adj->num_keys--;
	return root;
}

Node *delete_node (Value *key, BTreeInfo *tree_manager) {
	NodeValue *record = record_search(key, tree_manager->root);
	Node *key_leaf = leaf_search(key, tree_manager->root);
	if (record != NULL && key_leaf != NULL) {
		tree_manager->root = delete_entry(key_leaf, key, record, tree_manager);
		free(record = NULL);
	}
	return tree_manager->root;
}
