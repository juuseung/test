#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "helpers.h"
#include "dt.h"
#include "string.h"

Node *leaf_search(Node *root, Value *key) {
	Node *c = root;
	while (c != NULL && !c->leaf_flag) {
		int i = 0;
		while (i < c->num_keys) {
			if (!(is_greater(key, c->keys[i]) || is_equal(key, c->keys[i]))) break;
			i++;
		}
		c = c->ptrs[i];
	}
	return c;
}

NodeData *record_search(Node *root, Value *key) {
	int i = 0;
	Node *c = leaf_search(root, key);
	if (c == NULL) return NULL;
	for (i = 0; i < c->num_keys; i++) if (is_equal(c->keys[i], key)) break;
	if (i == c->num_keys) return NULL;
	return (NodeData *) c->ptrs[i];
}

void enqueue(BTreeManager *tree_manager, Node *new_node) {
	Node *c;
	if (tree_manager->queue == NULL) tree_manager->queue = new_node, tree_manager->queue->next = NULL;
	else {
		c = tree_manager->queue;
		while (c->next != NULL) c = c->next;
		c->next = new_node;
		new_node->next = NULL;
	}
}

Node *dequeue(BTreeManager *tree_manager) {
	Node *n = tree_manager->queue;
	tree_manager->queue = tree_manager->queue->next;
	n->next = NULL;
	return n;
}

int path(Node *root, Node *child) {
	Node *c = child;
	int i;
	for (i = 0; c != root; i++) c = c->parent;
	return i;
}

bool is_less(Value *k1, Value *k2) {
	if (k1->dt == DT_INT) {
		if (k1->v.intV < k2->v.intV) return TRUE;
	} else if (k1->dt == DT_FLOAT) {
		if (k1->v.floatV < k2->v.floatV) return TRUE;
	} else if (k1->dt == DT_STRING) {
		if (strcmp(k1->v.stringV, k2->v.stringV) == -1) return TRUE;
	}
	return FALSE;
}

bool is_greater(Value *k1, Value *k2) {
	if (k1->dt == DT_INT) {
		if (k1->v.intV > k2->v.intV) return TRUE;
	} else if (k1->dt == DT_FLOAT) {
		if (k1->v.floatV > k2->v.floatV) return TRUE;
	} else if (k1->dt == DT_STRING) {
		if (strcmp(k1->v.stringV, k2->v.stringV) == 1) return TRUE;
	}
	return FALSE;
}

bool is_equal(Value *k1, Value *k2) {
	if (k1->dt == DT_INT) {
		if (k1->v.intV == k2->v.intV) return TRUE;
	} else if (k1->dt == DT_FLOAT) {
		if (k1->v.floatV == k2->v.floatV) return TRUE;
	} else if (k1->dt == DT_STRING) {
		if (strcmp(k1->v.stringV, k2->v.stringV) == 0) return TRUE;
	} else if (k1->dt == DT_BOOL) {
		if (k1->v.boolV == k2->v.boolV) return TRUE;
	}
	return FALSE;
}