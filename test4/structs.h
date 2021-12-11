#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct Node {
	bool leaf_flag;
	Value **keys;
	void **ptrs;
	int num_keys;
	struct Node *parent_node, *next;
} Node;

typedef struct BTreeInfo {
	int order, num_nodes, num_entries;
	DataType key_type;
	Node *root, *queue;
	BM_PageHandle page_handler;
	BM_BufferPool buffer_pool_data;
} BTreeInfo;

typedef struct ScanManager {
	Node *node;
	int key_index, total_keys, order;
} ScanManager;

typedef struct NodeValue {
	RID rid;
} NodeValue;

#endif // STRUCTS_H