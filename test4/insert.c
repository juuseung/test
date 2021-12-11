#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "helpers.h"
#include "insert.h"

int get_left (Node *l_node, Node *parent_node){
	int leftIdx = 0;
	while ((parent_node->ptrs[leftIdx] != l_node) && (leftIdx <= parent_node->num_keys)) leftIdx++;
	return leftIdx;
}

Node *empty_leaf (BTreeInfo *tree_manager){
	Node *leaf = empty_node(tree_manager); leaf->leaf_flag = true;
	return leaf;
}

Node *empty_tree (Value *key, NodeValue *ptr, BTreeInfo *tree_manager) {
	Node *root = empty_leaf(tree_manager);
	root->parent_node = NULL, root->ptrs[tree_manager->order - 1] = NULL;
	root->keys[0] = key, root->ptrs[0] = ptr;
	root->num_keys++, tree_manager->num_entries++;
	return root;
}

Node *empty_node (BTreeInfo *tree_manager){
	Node *new_node = malloc(sizeof(Node));
	if (new_node == NULL) exit(RC_INSERT_FAILED);
	tree_manager->num_nodes++;
	new_node->keys = malloc((tree_manager->order-1) *sizeof(Value *)); new_node->ptrs = malloc((tree_manager->order) *sizeof(void *));
	if (new_node->keys == NULL) exit(RC_INSERT_FAILED); if (new_node->ptrs == NULL) exit(RC_INSERT_FAILED);
	new_node->parent_node = NULL, new_node->num_keys = 0, new_node->next = NULL, new_node->leaf_flag = false;
	return new_node;
}

Node *add_to_leaf (Value *key, NodeValue *ptr, Node *leaf, BTreeInfo *tree_manager) {
	int insert_idx = 0;
	tree_manager->num_entries++;
	while (insert_idx < leaf->num_keys && is_less(leaf->keys[insert_idx], key)) insert_idx++;
	for (int i = leaf->num_keys; i > insert_idx; i--) leaf->keys[i] = leaf->keys[i - 1], leaf->ptrs[i] = leaf->ptrs[i - 1];
	leaf->num_keys++;
	leaf->keys[insert_idx] = key, leaf->ptrs[insert_idx] = ptr;
	return leaf;
}

Node *add_to_parent (Node *l_node, Value *key, Node *r_node, BTreeInfo *tree_manager){
	Node *parent_node = l_node->parent_node;	
	if (parent_node == NULL) return add_to_empty_root(l_node, key, r_node, tree_manager);
	int left_idx = get_left(l_node,parent_node);
	if (parent_node->num_keys < tree_manager->order - 1) return add_to_node(parent_node, left_idx, key, r_node, tree_manager);
	return split_to_node(parent_node, key, r_node, tree_manager, left_idx);
}

NodeValue *new_record (RID *rid){
	NodeValue *record = (NodeValue *) malloc(sizeof(NodeValue));
	if (record != NULL) record->rid.slot = rid->slot, record->rid.page = rid ->page;
	return record;
}

Node *split_to_leaf (Value *key, NodeValue *ptr, Node *leaf, BTreeInfo *tree_manager) {
	void **tmp_ptr;
	Node *new_node = empty_leaf(tree_manager);
	Value **tmp_key;
	Value *new_key;
	tmp_key = malloc(sizeof(Value) * tree_manager->order);
	tmp_ptr = malloc(sizeof(void *) * tree_manager->order);
	int insert_idx = 0;
	while (insert_idx < tree_manager->order - 1 && is_less(leaf->keys[insert_idx], key)) insert_idx++;
	int i=0; int j=0;
	while(i<leaf->num_keys){
		if(j==insert_idx) j++;
		tmp_ptr[j] = leaf->ptrs[i]; tmp_key[j] = leaf->keys[i];
		i++; j++;
	}
	tmp_key[insert_idx] = key; tmp_ptr[insert_idx] = ptr;
	leaf->num_keys = 0;
	int split;
	if((tree_manager->order-1)%2 != 0) split=(tree_manager->order-1)/2 + 1;
	else split=(tree_manager->order-1)/2;
	i=0;
	while(i<split){
		leaf->keys[i] = tmp_key[i]; leaf->ptrs[i] = tmp_ptr[i];
		leaf->num_keys++; i++;
	}
	i=split; j=0;
	while(i<tree_manager->order){
		new_node->keys[j] = tmp_key[i]; new_node->ptrs[j] = tmp_ptr[i]; 
		new_node->num_keys++; i++; j++;
	}
	free(tmp_key = NULL); free(tmp_ptr = NULL);
	new_node->ptrs[tree_manager->order-1] = leaf->ptrs[tree_manager->order - 1];
	leaf->ptrs[tree_manager->order-1] = new_node;
	i=leaf->num_keys;j=new_node->num_keys;
	while(i<tree_manager->order-1){
		leaf->ptrs[i]=NULL; i++;
		new_node->ptrs[j]=NULL; j++;
	}
	tree_manager->num_entries++;
	new_node->parent_node = leaf->parent_node;
	new_key = (new_node->keys[0]);    
	return add_to_parent(leaf, new_key, new_node, tree_manager);
}

Node *split_to_node (Node *old_node, Value *key, Node *r_node, BTreeInfo *tree_manager, int left_idx) {
	Value *kp;
	Node *new_node, *child_node;
	Value **tmp_key;
	Node **tmp_ptr;
	int split;
	tmp_key = malloc(sizeof(Value *) * tree_manager->order);
	tmp_ptr = malloc(sizeof(Node *) * (tree_manager->order + 1));
	if (tmp_key || tmp_ptr == NULL) exit(RC_INSERT_FAILED);
	int i=0; int j=0;
	while(i<old_node->num_keys+1){
		if(j==left_idx+1) j++;
		tmp_ptr[j]=old_node->ptrs[i]; i++; j++;
	}
	i=0; j=0;
	while(i<old_node->num_keys){
		if(j==left_idx) j++;
		tmp_key[j]=old_node->keys[i]; i++; j++;
	}
	tmp_key[left_idx] = key, tmp_ptr[left_idx + 1] = r_node;
	if((tree_manager->order-1)%2 != 0) split=((tree_manager->order-1)/2 +1);
	else split=((tree_manager->order-1)/2);
	new_node = empty_node(tree_manager);
	old_node->num_keys = 0;
	i=0;
	while(i<split-1){
		old_node->ptrs[i]=tmp_ptr[i]; old_node->keys[i] = tmp_key[i];
		old_node->num_keys++; i++;
	}
	kp = tmp_key[split-1];
	old_node->ptrs[i] = tmp_ptr[i];
	++i; j=0;
	while(i<tree_manager->order){
		new_node->keys[j]=tmp_key[i]; new_node->ptrs[j]=tmp_ptr[i];
		new_node->num_keys++; i++; j++;
	}
	new_node->ptrs[j] = tmp_ptr[i];
	free(tmp_key = NULL); free(tmp_ptr = NULL);
	new_node->parent_node = old_node->parent_node;
	i=0;
	while(i<new_node->num_keys+1){
		child_node->parent_node=new_node; child_node=new_node->ptrs[i]; i++;
	}
	tree_manager->num_entries++;
	return add_to_parent(old_node, kp, new_node, tree_manager);
}

Node *add_to_node (Node *parent_node, int left_idx, Value *key, Node *r_node, BTreeInfo *tree_manager){
	for (int i = parent_node->num_keys; i > left_idx; i--) parent_node->keys[i] = parent_node->keys[i - 1], parent_node->ptrs[i + 1] = parent_node->ptrs[i];
	parent_node->keys[left_idx] = key, parent_node->ptrs[left_idx + 1] = r_node;
	parent_node->num_keys++;
	return tree_manager->root;
}

Node *add_to_empty_root (Node *l_node, Value *key, Node *r_node, BTreeInfo *tree_manager){
	Node *root = empty_node(tree_manager);
	root->num_keys++; root->keys[0] = key;
	root->ptrs[0] = l_node, root->ptrs[1] = r_node, root->parent_node = NULL;	
	l_node->parent_node = root, r_node->parent_node = root;
	return root;
}