#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "helpers.h"
#include "string.h"

int path (Node *child, Node *root) {
	int c;
	for (c = 0; child != root; c++) child = child->parent_node;
	return c;
}

bool is_equal (Value *k1, Value *k2) {
	if (k1->dt == DT_FLOAT) {
		if (k1->v.floatV == k2->v.floatV) return TRUE;
	} else if (k1->dt == DT_STRING) {
		if (strcmp(k1->v.stringV, k2->v.stringV) == 0) return TRUE;
	} else if (k1->dt == DT_INT) {
		if (k1->v.intV == k2->v.intV) return TRUE;
	} else if (k1->dt == DT_BOOL) {
		if (k1->v.boolV == k2->v.boolV) return TRUE;
	}
	return FALSE;
}

bool is_greater (Value *k1, Value *k2) {
	if (k1->dt == DT_FLOAT) {
		if (k1->v.floatV > k2->v.floatV) return TRUE;
	} else if (k1->dt == DT_STRING) {
		if (strcmp(k1->v.stringV, k2->v.stringV) == 1) return TRUE;
	} else if (k1->dt == DT_INT) {
		if (k1->v.intV > k2->v.intV) return TRUE;
	}
	return FALSE;
}

bool is_less (Value *k1, Value *k2) {
	if (k1->dt == DT_FLOAT) {
		if (k1->v.floatV < k2->v.floatV) return TRUE;
	} else if (k1->dt == DT_STRING) {
		if (strcmp(k1->v.stringV, k2->v.stringV) == -1) return TRUE;
	} else if (k1->dt == DT_INT) {
		if (k1->v.intV < k2->v.intV) return TRUE;
	}
	return FALSE;
}

void enqueue (Node *new_node, BTreeInfo *tree_manager) {
	if (tree_manager->queue != NULL) {
		while (tree_manager->queue->next != NULL) tree_manager->queue = tree_manager->queue->next;
		tree_manager->queue->next = new_node;
		new_node->next = NULL;
	}
	else tree_manager->queue = new_node, tree_manager->queue->next = NULL;
}

Node *dequeue (BTreeInfo *tree_manager) {
	tree_manager->queue = tree_manager->queue->next;
	tree_manager->queue->next = NULL;
	return tree_manager->queue;
}

NodeValue *record_search (Value *key, Node *root) {
	Node *current = leaf_search(key, root);
	if (current == NULL) return NULL;
	int i = 0;
	for (i = 0; i < current->num_keys; i++) if (is_equal(current->keys[i], key)) break;
	if (i == current->num_keys) return NULL;
	return (NodeValue *) current->ptrs[i];
}

Node *leaf_search (Value *key, Node *root) {
	while (root != NULL && !root->leaf_flag) {
		int i = 0;
		while (i < root->num_keys) {
			if (!(is_equal(key, root->keys[i]) || is_greater(key, root->keys[i]))) break;
			i++;
		}
		root = root->ptrs[i];
	}
	return root;
}