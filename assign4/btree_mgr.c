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

BTreeManager *tree_manager = NULL;

RC initIndexManager(void *mgmtData) {
	initStorageManager();
	return RC_OK;
}

RC shutdownIndexManager() {
	tree_manager = NULL;
	return RC_OK;
}

RC createBtree(char *idxId, DataType key_type, int n) {

	int maxNodes = PAGE_SIZE / sizeof(Node);

	if (n > maxNodes) return RC_EXCEED_MAX;

	tree_manager = (BTreeManager *) malloc(sizeof(BTreeManager));
	tree_manager->order = n + 2;
	tree_manager->num_nodes = 0, tree_manager->num_entries = 0;
	tree_manager->root = NULL, tree_manager->queue = NULL;
	tree_manager->key_type = key_type;

	BM_BufferPool *bm = (BM_BufferPool *) malloc(sizeof(BM_BufferPool));
	tree_manager->buffer_pool_data = *bm;

	SM_FileHandle fileHandler;
	RC result = RC_OK;

	char data[PAGE_SIZE];

	if ((result = createPageFile(idxId)) != RC_OK) return result;
	if ((result = openPageFile(idxId, &fileHandler)) != RC_OK) return result;
	if ((result = writeBlock(0, &fileHandler, data)) != RC_OK) return result;
	if ((result = closePageFile(&fileHandler)) != RC_OK) return result;

	return result;
}

RC openBtree(BTreeHandle **tree, char *idxId) {

	*tree = (BTreeHandle *) malloc(sizeof(BTreeHandle));
	(*tree)->mgmtData = tree_manager;

	RC result = RC_OK;
	
	if ((result = initBufferPool(&tree_manager->buffer_pool_data, idxId, 1000, RS_FIFO, NULL)) != RC_OK) return result;

	return result;
}

RC closeBtree(BTreeHandle *tree) {
	BTreeManager *tree_manager = (BTreeManager*) tree->mgmtData;
	markDirty(&tree_manager->buffer_pool_data, &tree_manager->page_handler);
	shutdownBufferPool(&tree_manager->buffer_pool_data);
	free(tree_manager), free(tree);
	return RC_OK;
}

RC deleteBtree(char *idxId) {
	RC result;
	result = destroyPageFile(idxId);
	if(result != RC_OK) return result;
	else return RC_OK;
}

RC insertKey(BTreeHandle *tree, Value *key, RID rid) {

	BTreeManager *tree_manager = (BTreeManager *) tree->mgmtData;
	NodeData *ptr;
	Node *leaf;

	int bTreeOrder = tree_manager->order;

	if (record_search(tree_manager->root, key) != NULL) {
		printf("\n insertKey :: KEY EXISTS");
		return RC_IM_KEY_ALREADY_EXISTS;
	}

	ptr = new_record(&rid);

	if (tree_manager->root == NULL) {
		tree_manager->root = empty_tree(tree_manager, key, ptr);
		return RC_OK;
	}

	leaf = leaf_search(tree_manager->root, key);

	if (leaf->num_keys < bTreeOrder - 1) leaf = add_to_leaf(key, ptr, leaf, tree_manager);
	else tree_manager->root = split_to_leaf(key, ptr, leaf, tree_manager);

	return RC_OK;
}

extern RC findKey(BTreeHandle *tree, Value *key, RID *result) {
	BTreeManager *tree_manager = (BTreeManager *) tree->mgmtData;
	NodeData *r = record_search(tree_manager->root, key);
	if (r == NULL) return RC_IM_KEY_NOT_FOUND;
	*result = r->rid;
	return RC_OK;
}

RC getNumNodes(BTreeHandle *tree, int *result) {
	BTreeManager *tree_manager = (BTreeManager *) tree->mgmtData;
	*result = tree_manager->num_nodes;
	return RC_OK;
}

RC getNumEntries(BTreeHandle *tree, int *result) {
	BTreeManager *tree_manager = (BTreeManager *) tree->mgmtData;
	*result = tree_manager->num_entries;
	return RC_OK;
}

RC getKeyType(BTreeHandle *tree, DataType *result) {
	BTreeManager *tree_manager = (BTreeManager *) tree->mgmtData;
	*result = tree_manager->key_type;
	return RC_OK;
}

RC deleteKey(BTreeHandle *tree, Value *key) {
	BTreeManager *tree_manager = (BTreeManager *) tree->mgmtData;
	tree_manager->root = delete_node(tree_manager, key);
	return RC_OK;
}

RC openTreeScan(BTreeHandle *tree, BT_ScanHandle **handle) {

	BTreeManager *tree_manager = (BTreeManager *) tree->mgmtData;
	Node *node = tree_manager->root;

	ScanManager *scanmeta = malloc(sizeof(ScanManager));
	*handle = malloc(sizeof(BT_ScanHandle));

	if (tree_manager->root == NULL) return RC_EMPTY;

	while (!node->leaf_flag) node = node->ptrs[0];

	scanmeta->key_index = 0;
	scanmeta->total_keys = node->num_keys;
	scanmeta->node = node;
	scanmeta->order = tree_manager->order;
	(*handle)->mgmtData = scanmeta;

	return RC_OK;
}

RC nextEntry(BT_ScanHandle *handle, RID *result) {

	ScanManager *scanmeta = (ScanManager *) handle->mgmtData;
	int key_index = scanmeta->key_index, total_keys = scanmeta->total_keys, bTreeOrder = scanmeta->order;
	Node *node = scanmeta->node;
	RID rid;

	if (node == NULL) return RC_IM_NO_MORE_ENTRIES;

	if(key_index >= total_keys){
		if(node->ptrs[bTreeOrder-1] == NULL) return RC_IM_NO_MORE_ENTRIES;
		node=node->ptrs[bTreeOrder-1];
		scanmeta->key_index=1; scanmeta->node=node; scanmeta->total_keys=node->num_keys;
		rid=((NodeData *)node->ptrs[0])->rid;
	}else{
		rid=((NodeData *)node->ptrs[key_index])->rid;
		scanmeta->key_index++;
	}
	*result = rid;

	return RC_OK;
}

extern RC closeTreeScan(BT_ScanHandle *handle) {
	handle->mgmtData = NULL;
	free(handle);
	return RC_OK;
}

extern char *printTree(BTreeHandle *tree) {
	BTreeManager *tree_manager = (BTreeManager *) tree->mgmtData;
	printf("\nPRINTING TREE:\n");
	Node *n = NULL;
	int i = 0, rank = 0, new_rank = 0;

	if (tree_manager->root == NULL) {
		printf("Empty tree.\n");
		return '\0';
	}
	tree_manager->queue = NULL;
	enqueue(tree_manager, tree_manager->root);
	while (tree_manager->queue != NULL) {
		n = dequeue(tree_manager);
		if (n->parent != NULL && n == n->parent->ptrs[0]) {
			new_rank = path(tree_manager->root, n);
			if (new_rank != rank) {
				rank = new_rank;
				printf("\n");
			}
		}

		for (i = 0; i < n->num_keys; i++) {
			if (tree_manager->key_type == DT_INT) printf("%d ", (*n->keys[i]).v.intV);
			else if (tree_manager->key_type == DT_FLOAT) printf("%.02f ", (*n->keys[i]).v.floatV);
			else if (tree_manager->key_type == DT_STRING) printf("%s ", (*n->keys[i]).v.stringV);
			else if (tree_manager->key_type == DT_BOOL) printf("%d ", (*n->keys[i]).v.boolV);
			printf("(%d - %d) ", ((NodeData *) n->ptrs[i])->rid.page, ((NodeData *) n->ptrs[i])->rid.slot);
		}

		if (!n->leaf_flag) for (i = 0; i <= n->num_keys; i++) enqueue(tree_manager, n->ptrs[i]);
		printf("| ");
	}

	printf("\n");
	return '\0';
}
