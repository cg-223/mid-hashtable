#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <hashtable.h>


//allocate memory and initialize to 0
void* xmalloc(size_t amount) {
	void* toRet = malloc(amount);
	if (toRet == NULL) {
		perror("oom");
		exit(1);
	}
	memset(toRet, 0, amount);
	return toRet;
}


//creates a new hashtable with default values
#define spawn_hashtable() create_hashtable(DEFAULT_CAPACITY)
//hashtable* spawn_hashtable() {
//	return create_hashtable(DEFAULT_CAPACITY);
//}


//create a hashtable with specified values
hashtable* create_hashtable(size_t capacity) {
	hashtable* myHash = (hashtable*)xmalloc(sizeof(hashtable));
	myHash->array = (head**)xmalloc(sizeof(head*) * capacity);
	myHash->capacity = capacity;
	for (int i = 0; i < capacity; i++) {
		myHash->array[i] = (head*)xmalloc(sizeof(head));
		myHash->array[i]->first = NULL;
		myHash->array[i]->treeLen = 0;
	}
	clear_hashtable(myHash);
	return myHash;
}


//clear all values from the hashtable (sets all nodes to NULL). does not free any nodes. may lead to leaks.
//all heads stay
void clear_hashtable(hashtable* toClear) {
	for (int i = 0; i < toClear->capacity; i++) {
		toClear->array[i]->first = NULL;
		toClear->array[i]->treeLen = 0;
	}
}

//recursively wipe all nodes in a chain
void wipe_node(node* toWipe) {
	if (toWipe == NULL) {
		return;
	}
	wipe_node(toWipe->next);
	free(toWipe);
}

//recursively wipe all nodes in a chain, and free the attached data
void wipe_node_and_data(node* toWipe) {
	if (toWipe == NULL) {
		return;
	}
	wipe_node_and_data(toWipe->next);
	if (toWipe->data != NULL) {
		free(toWipe->data);
	}
	free(toWipe);
}

//wipe all values from the hashtable. does not free data, but does free all nodes.
void wipe_hashtable(hashtable* toWipe) {
	for (int i = 0; i < toWipe->capacity; i++) {
		wipe_node(toWipe->array[i]->first);
		toWipe->array[i]->first = NULL;
	}
}



//frees any and all pointer-stored data in the hashtable. guaranteed to never leak any data
void wipe_hashtable_and_data(hashtable* toWipe) {
	for (int i = 0; i < toWipe->capacity; i++) {
		wipe_node_and_data(toWipe->array[i]->first);
		toWipe->array[i]->first = NULL;
	}
}


void resize_hashtable(hashtable* toResize) {
	resize_hashtable_specific(toResize, (size_t)floor((double)toResize->capacity * DEFAULT_RESIZE));
}



void resize_hashtable_specific(hashtable* toResize, size_t new_capacity) {
	head** realloced = xmalloc(new_capacity * sizeof(head*));
	memcpy(realloced, toResize->array, (toResize->capacity - 1) * sizeof(head*));
	clear_hashtable(toResize);
	if (realloced == NULL) {
		perror("oom");
		exit(1);
	}
	else {
		size_t oldSize = toResize->capacity;
		toResize->capacity = new_capacity;
		for (int i = 0; i < oldSize; i++) {
			node* curNode = realloced[i]->first;
			while (curNode != NULL) {
				insert_node_into_hashtable(toResize, curNode);
				curNode = curNode->next;
			}
		}
	}
}

node* create_node(void* key, size_t keysize, void* data, size_t size) {
	node* myNode = xmalloc(sizeof(node));
	myNode->next = NULL;
	myNode->size = size;
	myNode->data = data;

	myNode->keysize = keysize;
	myNode->key = key;

	return myNode;
}

void insert_node_into_hashtable(hashtable* table, node* addThis) {
	size_t hashed = hash_this_node(addThis, table->capacity);
	head* head = table->array[hashed];
	size_t depth = 0;
	addThis->next = head->first;
	head->treeLen++;
	head->first = addThis;
#ifndef DONT_RESIZE
	if (depth > LIST_LENGTH_RESIZE_THRESH) {
		resize_hashtable(table);
	}
#endif
}

void insert_data_into_hashtable(hashtable* table, void* key, size_t keysize, void* data, size_t size) {
	node* ourNode = create_node(key, keysize, data, size);
	insert_node_into_hashtable(table, ourNode);
}

node* lookup_key_in_hashtable(hashtable* table, void* key, size_t keysize) {
	size_t hashed = hash(key, keysize, table->capacity);
	node* at = table->array[hashed]->first;
	while (at != NULL) {
		if (!compareData(at->key, at->keysize, key, keysize))
			return at;

		at = at->next;
	}
	return NULL;
}

node* lookup_string_in_hashtable(hashtable* table, char* key) {
	return lookup_key_in_hashtable(table, key, strlen(key) + 1);
}

void delete_key_from_hashtable(hashtable* table, void* key, size_t keysize) {
	size_t pos = hash(key, keysize, table->capacity);
	node* at = table->array[pos]->first;
	node* before = NULL;
	while (at != NULL) {
		if (!compareData(key, keysize, at->key, at->keysize)) {
			if (before != NULL) {
				table->array[pos]->treeLen--;
				before->next = at->next;
			}
			else {
				table->array[pos]->first = at->next;
			}
		}
		before = at;
		at = at->next;
	}
}

void delete_key_string_from_hashtable(hashtable* table, char* key) {
	delete_key_from_hashtable(table, key, strlen(key)+1);
}

//classic -1 / 0 / 1
int compareData(void* data1, size_t size1, void* data2, size_t size2) {
	if (size1 != size2)
		return size1 > size2;

	char* ptr1 = (char*)data1;
	char* ptr2 = (char*)data2;

	for (int i = 0; i < size1; i++) {
		if (ptr1[i] != ptr2[i])
			return ptr1[i] > ptr2[i]*2-1;

	}
	return 0;
}

size_t hash(void* key, size_t len, size_t capacity) {
	unsigned long hash = 0;
	char* charData = (char*)key;
	for (int i = 0; i < len; i++) {
		hash = charData[i] + (hash << 6) + (hash << 16) - hash;
	}

	return hash % capacity;
}

size_t hash_this_node(node* toHash, size_t capacityOfHashTable) {
	return hash(toHash->key, toHash->keysize, capacityOfHashTable);
}

int main() {
	test_hashtable();

	return 0;
}

void test_hashtable() {
	hashtable* myTable = spawn_hashtable();
	char myKey[] = "key";
	char myVal[] = "value";
	insert_data_into_hashtable(myTable, myKey, strlen(myKey) + 1, myVal, strlen(myVal) + 1);
	node* myNode = lookup_string_in_hashtable(myTable, "key");
	if (myNode == NULL) {
		printf("Node %s not found...\n", myKey);
	}

	for (int i = 0; i <= 10000000; i++) {
		char* a = xmalloc(32);
		_itoa_s(i, a, 32, 10);

		char* b = xmalloc(32);
		_itoa_s(i*2, b, 32, 10);
		
		insert_data_into_hashtable(myTable, a, (size_t)strlen(a)+1, b, (size_t)strlen(a) + 1);
		node* myNode = lookup_string_in_hashtable(myTable, a);
		if (myNode == NULL) {
			printf("Node %s not found...\n", a);
		}
		else if (b != myNode->data) {
			printf("Node mismatch...\n");
		}
		if (i % 5 == 0) {
			delete_key_from_hashtable(myTable, a, (size_t)strlen(a) + 1);
		}
#ifdef HEAVY_LOOKUP_DEBUG
		for (int i = 0; i <= HEAVY_LOOKUP_DEBUG; i++) {
			lookup_string_in_hashtable(myTable, "hello");
			lookup_string_in_hashtable(myTable, a);
		}
#endif
	}

	wipe_hashtable_and_data(myTable);
}
