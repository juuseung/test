#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct Node {
	bool leaf_flag;
	Value **keys;
	void **ptrs;
	int num_keys;
	struct Node *parent, *next;
} Node;

typedef struct BTreeManager {
	BM_BufferPool buffer_pool_data;
	BM_PageHandle page_handler;
	int order, num_nodes, num_entries;
	Node *root, *queue;
	DataType key_type;
} BTreeManager;

typedef struct ScanManager {
	int key_index, total_keys, order;
	Node *node;
} ScanManager;

typedef struct NodeData {
	RID rid;
} NodeData;

#endif // STRUCTS_H