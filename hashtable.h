#pragma once
typedef struct node {
	void* data;
	size_t size;

	void* key;
	size_t keysize;

	struct node* next;
} node;

typedef struct hashtable {
	struct node** array;
	size_t capacity; //in pointers
} hashtable;

#define DEFAULT_CAPACITY 16

/*
resize options:
DEFAULT_RESIZE: List slots multiplied by DEFAULT_RESIZE
LIST_LENGTH_RESIZE_THRESH: When a node is inserted, if it's nested more than LIST_LENGTH_RESIZE_THRESH times, resize the hashtable (this DOES rehash every element)
*/
#define DEFAULT_RESIZE 1.5
#define LIST_LENGTH_RESIZE_THRESH 4

void wipe_hashtable(hashtable* toWipe);
void resize_hashtable_specific(hashtable* toResize, size_t new_capacity);
void insert_node_into_hashtable(hashtable* table, node* addThis);
size_t hash_this_node(node* toHash, size_t capacityOfHashTable);
size_t hash(void* key, size_t len, size_t capacity);
void test_hashtable();
void clear_hashtable(hashtable* toClear);
