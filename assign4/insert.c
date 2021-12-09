#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "helpers.h"
#include "insert.h"
#include "dt.h"

NodeData *new_record(RID *rid) {
	NodeData *record = (NodeData *) malloc(sizeof(NodeData));
	if (record != NULL) record->rid.slot = rid->slot, record->rid.page = rid ->page;
	return record;
}

Node *empty_tree(BTreeManager *tree_manager, Value *key, NodeData *ptr) {
	Node *root = empty_leaf(tree_manager);
	int bTreeOrder = tree_manager->order;
	root->parent = NULL, root->ptrs[bTreeOrder - 1] = NULL;
	root->keys[0] = key, root->ptrs[0] = ptr;
	root->num_keys++, tree_manager->num_entries++;
	return root;
}

Node *add_to_leaf(Value *key, NodeData *ptr, Node *leaf, BTreeManager *tree_manager) {

	int insertion_point = 0;
	tree_manager->num_entries++;
	while (insertion_point < leaf->num_keys && is_less(leaf->keys[insertion_point], key)) insertion_point++;
	for (int i = leaf->num_keys; i > insertion_point; i--)
		leaf->keys[i] = leaf->keys[i - 1], leaf->ptrs[i] = leaf->ptrs[i - 1];
	leaf->keys[insertion_point] = key, leaf->ptrs[insertion_point] = ptr;
	leaf->num_keys++;
	return leaf;
}

Node *split_to_leaf(Value *key, NodeData *ptr, Node *leaf, BTreeManager *tree_manager) {

	Node *new_leaf;
	Value **temp_keys;
	void **temp_pts;
	int insertion_index, split, i, j;
	Value *new_key;

	new_leaf = empty_leaf(tree_manager);
	int bTreeOrder = tree_manager->order;

	temp_keys = malloc(bTreeOrder *sizeof(Value));
	temp_pts = malloc(bTreeOrder *sizeof(void *));

	insertion_index = 0;
	while (insertion_index < bTreeOrder - 1 && is_less(leaf->keys[insertion_index], key)) insertion_index++;

	i=0; j=0;
	while(i<leaf->num_keys){
		if(j==insertion_index) j++;
		temp_keys[j] = leaf->keys[i]; temp_pts[j] = leaf->ptrs[i];
		i++; j++;
	}

	temp_keys[insertion_index] = key; temp_pts[insertion_index] = ptr;

	leaf->num_keys = 0;

	if((bTreeOrder-1)%2 != 0) split=(bTreeOrder-1)/2 + 1;
	else split=(bTreeOrder-1)/2;

	i=0;
	while(i<split){
		leaf->ptrs[i] = temp_pts[i]; leaf->keys[i] = temp_keys[i];
		leaf->num_keys++; i++;
	}

	i=split; j=0;
	while(i<bTreeOrder){
		new_leaf->ptrs[j] = temp_pts[i]; new_leaf->keys[j] = temp_keys[i];
		new_leaf->num_keys++; i++; j++;
	}

	free(temp_pts); free(temp_keys);

	new_leaf->ptrs[bTreeOrder - 1] = leaf->ptrs[bTreeOrder - 1];
	leaf->ptrs[bTreeOrder - 1] = new_leaf;

	i=leaf->num_keys;j=new_leaf->num_keys;
	while(i<bTreeOrder-1){
		leaf->ptrs[i]=NULL; i++;
		new_leaf->ptrs[j]=NULL; j++;
	}

	new_leaf->parent = leaf->parent;
	new_key = (new_leaf->keys[0]);
	tree_manager->num_entries++;
    
	return add_to_parent(tree_manager, leaf, new_key, new_leaf);
}

Node *split_to_node(BTreeManager *tree_manager, Node *old_node, int left_index, Value *key, Node *right) {

	int i, j, split;
	Value *k_prime;
	Node *new_node, *child;
	Value **temp_keys;
	Node **temp_pts;

	int bTreeOrder = tree_manager->order;

	temp_keys = malloc(bTreeOrder *sizeof(Value *));
	temp_pts = malloc((bTreeOrder + 1) *sizeof(Node *));
	if (temp_keys || temp_pts == NULL) exit(RC_INSERT_FAILED);

	i=0; j=0;
	while(i<old_node->num_keys+1){
		if(j==left_index+1) j++;
		temp_pts[j]=old_node->ptrs[i]; i++; j++;
	}

	i=0; j=0;
	while(i<old_node->num_keys){
		if(j==left_index) j++;
		temp_keys[j]=old_node->keys[i]; i++; j++;
	}

	temp_keys[left_index] = key, temp_pts[left_index + 1] = right;

	if((bTreeOrder-1)%2 != 0) split=(bTreeOrder-1)/2 + 1;
	else split=(bTreeOrder-1)/2;

	new_node = empty_node(tree_manager);
	old_node->num_keys = 0;

	i=0;
	while(i<split-1){
		old_node->ptrs[i]=temp_pts[i]; old_node->keys[i] = temp_keys[i];
		old_node->num_keys++; i++;
	}

	k_prime = temp_keys[split-1];
	old_node->ptrs[i] = temp_pts[i];

	++i; j=0;
	while(i<bTreeOrder){
		new_node->keys[j]=temp_keys[i]; new_node->ptrs[j]=temp_pts[i];
		new_node->num_keys++; i++; j++;
	}
	new_node->ptrs[j] = temp_pts[i];

	free(temp_pts), free(temp_keys);

	new_node->parent = old_node->parent;

	i=0;
	while(i<new_node->num_keys+1){
		child->parent=new_node; child=new_node->ptrs[i]; i++;
	}

	tree_manager->num_entries++;
	return add_to_parent(tree_manager, old_node, k_prime, new_node);
}

Node *add_to_parent(BTreeManager *tree_manager, Node *left, Value *key, Node *right) {
	Node *parent = left->parent;
	int bTreeOrder = tree_manager->order;
	int left_index;
	if (parent == NULL) return add_to_empty_root(tree_manager, left, key, right);
	left_index = get_left(parent, left);
	if (parent->num_keys < bTreeOrder - 1) return add_to_node(tree_manager, parent, left_index, key, right);
	return split_to_node(tree_manager, parent, left_index, key, right);
}

int get_left(Node *parent, Node *left) {
	int left_index = 0;
	while (left_index <= parent->num_keys && parent->ptrs[left_index] != left) left_index++;
	return left_index;
}

Node *add_to_node(BTreeManager *tree_manager, Node *parent, int left_index, Value *key, Node *right) {
	for (int i = parent->num_keys; i > left_index; i--) parent->keys[i] = parent->keys[i - 1], parent->ptrs[i + 1] = parent->ptrs[i];
	parent->keys[left_index] = key, parent->ptrs[left_index + 1] = right;
	parent->num_keys++;
	return tree_manager->root;
}

Node *add_to_empty_root(BTreeManager *tree_manager, Node *left, Value *key, Node *right) {
	Node *root = empty_node(tree_manager);
	root->keys[0] = key, root->ptrs[0] = left, root->ptrs[1] = right;
	root->num_keys++;
	root->parent = NULL, left->parent = root, right->parent = root;
	return root;
}

Node *empty_node(BTreeManager *tree_manager) {
	Node *new_node = malloc(sizeof(Node));
	int bTreeOrder = tree_manager->order;
	tree_manager->num_nodes++;
	if (new_node == NULL) exit(RC_INSERT_FAILED);
	new_node->keys = malloc((bTreeOrder - 1) *sizeof(Value *)); if (new_node->keys == NULL) exit(RC_INSERT_FAILED);
	new_node->ptrs = malloc(bTreeOrder *sizeof(void *)); if (new_node->ptrs == NULL) exit(RC_INSERT_FAILED);
	new_node->leaf_flag = false, new_node->num_keys = 0, new_node->parent = NULL, new_node->next = NULL;
	return new_node;
}

Node *empty_leaf(BTreeManager *tree_manager) {
	Node *leaf = empty_node(tree_manager);
	leaf->leaf_flag = true;
	return leaf;
}
