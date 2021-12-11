#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "dberror.h"
#include "btree_mgr.h"
#include "insert.h"
#include "delete.h"
#include "helpers.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"
#include "tables.h"

BTreeInfo *tree_manager = NULL;

RC initIndexManager (void *mgmtData) {
	initStorageManager();
	return RC_OK;
}

RC shutdownIndexManager() {
	free(tree_manager = NULL);
	return RC_OK;
}

RC createBtree (char *idxId, DataType key_type, int n) {
	if (n > (PAGE_SIZE / sizeof(Node))) return RC_EXCEED_MAX;
	char data[PAGE_SIZE];
	SM_FileHandle f;
	tree_manager = (BTreeInfo *) malloc(sizeof(BTreeInfo));
	tree_manager->root = NULL, tree_manager->queue = NULL;
	tree_manager->key_type = key_type;
	tree_manager->num_entries = 0, tree_manager->num_nodes = 0;
	tree_manager->order = n+2;
	BM_BufferPool *bm = (BM_BufferPool *) malloc(sizeof(BM_BufferPool));
	tree_manager->buffer_pool_data = *bm;
	createPageFile(idxId);
	openPageFile(idxId,&f);
	writeBlock(0,&f,data);
	return closePageFile(&f);
}

RC openBtree (BTreeHandle **tree, char *idxId) {
	*tree = (BTreeHandle *) malloc(sizeof(BTreeHandle));
	(*tree)->mgmtData = tree_manager;
	return initBufferPool(&tree_manager->buffer_pool_data, idxId, 1000, RS_FIFO, NULL);
}

RC closeBtree (BTreeHandle *tree) {
	BTreeInfo *tree_manager = (BTreeInfo*) tree->mgmtData;
	markDirty(&tree_manager->buffer_pool_data, &tree_manager->page_handler);
	shutdownBufferPool(&tree_manager->buffer_pool_data);
	free(tree_manager = NULL), free(tree = NULL);
	return RC_OK;
}

RC deleteBtree (char *idxId) { return destroyPageFile(idxId); }

RC insertKey (BTreeHandle *tree, Value *key, RID rid) {
	BTreeInfo *tree_manager = (BTreeInfo *) tree->mgmtData;
	if (record_search(key, tree_manager->root) != NULL) return RC_IM_KEY_ALREADY_EXISTS;
	Node *leaf;
	NodeValue *ptr = new_record(&rid);
	if (tree_manager->root == NULL) {
		tree_manager->root = empty_tree(key, ptr, tree_manager); return RC_OK;
	}
	leaf = leaf_search(key, tree_manager->root);
	if(leaf->num_keys>=tree_manager->order-1) tree_manager->root = split_to_leaf(key, ptr, leaf, tree_manager);
	else leaf = add_to_leaf(key, ptr, leaf, tree_manager);
	return RC_OK;
}

extern RC findKey (BTreeHandle *tree, Value *key, RID *result) {
	BTreeInfo *tree_manager = (BTreeInfo *) tree->mgmtData;
	NodeValue *r = record_search(key, tree_manager->root);
	if(r!=NULL){ *result=r->rid; return RC_OK; }
	return RC_IM_KEY_NOT_FOUND;
}

RC getNumNodes (BTreeHandle *tree, int *result) {
	BTreeInfo *tree_manager = (BTreeInfo *) tree->mgmtData;
	*result = tree_manager->num_nodes;
	return RC_OK;
}

RC getNumEntries (BTreeHandle *tree, int *result) {
	BTreeInfo *tree_manager = (BTreeInfo *) tree->mgmtData;
	*result = tree_manager->num_entries;
	return RC_OK;
}

RC getKeyType (BTreeHandle *tree, DataType *result) {
	BTreeInfo *tree_manager = (BTreeInfo *) tree->mgmtData;
	*result = tree_manager->key_type;
	return RC_OK;
}

RC deleteKey (BTreeHandle *tree, Value *key) {
	BTreeInfo *tree_manager = (BTreeInfo *) tree->mgmtData;
	tree_manager->root = delete_node(key, tree_manager);
	return RC_OK;
}

RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle) {
	BTreeInfo *tree_manager = (BTreeInfo *) tree->mgmtData;
	Node *node = tree_manager->root;
	if (tree_manager->root==NULL) return RC_EMPTY;
	while (!node->leaf_flag) node = node->ptrs[0];
	ScanManager *scan = malloc(sizeof(ScanManager));
	scan->node = node;
	scan->key_index = 0;
	scan->order = tree_manager->order;
	scan->total_keys = node->num_keys;
	*handle = malloc(sizeof(BT_ScanHandle));
	(*handle)->mgmtData = scan;
	return RC_OK;
}

RC nextEntry (BT_ScanHandle *handle, RID *result) {
	RID rid;
	ScanManager *scan = (ScanManager *) handle->mgmtData;
	Node *node = scan->node;
	if (node == NULL) return RC_IM_NO_MORE_ENTRIES;
	if (scan->key_index >= scan->total_keys){
		if (node->ptrs[scan->order-1] == NULL) return RC_IM_NO_MORE_ENTRIES;
		node=node->ptrs[scan->order-1]; scan->node=node;
		scan->key_index=1; scan->total_keys=node->num_keys;
		rid=((NodeValue *)node->ptrs[0])->rid;
		*result = rid;
	} else {
		scan->key_index++;
		rid=((NodeValue *)node->ptrs[scan->key_index-1])->rid;
		*result = rid;
	}
	return RC_OK;
}

RC closeTreeScan (BT_ScanHandle *handle) {
	free(handle->mgmtData = NULL);
	free(handle = NULL);
	return RC_OK;
}

char *printTree (BTreeHandle *tree) {
	printf("\nPRINTING TREE:\n");
	BTreeInfo *tree_manager = (BTreeInfo *) tree->mgmtData;
	if (tree_manager->root == NULL) {
		printf("Empty tree.\n");
		return '\0';
	}
	tree_manager->queue = NULL;
	enqueue(tree_manager->root, tree_manager);
	Node *n = NULL;
	int rank = 0, newRank = 0;
	while (tree_manager->queue!=NULL) {
		n = dequeue(tree_manager);
		if ((n == n->parent_node->ptrs[0])&&(n->parent_node!=NULL)) {
			newRank = path(n, tree_manager->root);
			if (newRank != rank) {
				rank=newRank;
				printf("\n");
			}
		}
		for (int i=0;i<n->num_keys;i++) {
			if (tree_manager->key_type == DT_FLOAT) printf("%.02f ",(*n->keys[i]).v.floatV);
			else if (tree_manager->key_type == DT_INT) printf("%d ",(*n->keys[i]).v.intV);
			else if (tree_manager->key_type == DT_BOOL) printf("%d ",(*n->keys[i]).v.boolV);
			else if (tree_manager->key_type == DT_STRING) printf("%s ",(*n->keys[i]).v.stringV);
			printf("(%d - %d) ", ((NodeValue *) n->ptrs[i])->rid.page, ((NodeValue *)n->ptrs[i])->rid.slot);
		}
		if (!n->leaf_flag) for (int i=0; i<=n->num_keys;i++) enqueue(n->ptrs[i], tree_manager);
		printf("| ");
	}
	printf("\n");
	return '\0';
}