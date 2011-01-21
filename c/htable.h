#pragma once

#include <stdbool.h>

typedef struct {
	void *user_data;
	int size, item_size, key_size;
	int *links, *f_links, *b_links;
	int cur_f, cur_b;
	bool *occupied;
	void (*keyer)(void *item, void *key);
	int (*hasher)(void *key, int size);
	void (*getter)(void *user_data, int pos, void *item);
	void (*setter)(void *user_data, int pos, void *item);
} htable_t;

extern htable_t *htable_create(int size, int item_size, int key_size, void *user_data, void (*keyer)(void *item, void *key), int (*hasher)(void *key, int size), void (*getter)(void *user_data, int pos, void *item), void (*setter)(void *user_data, int pos, void *item));
extern void htable_delete(htable_t *ht);
extern int htable_add(htable_t *ht, void *item);
extern void htable_print_table(htable_t *ht);
extern int htable_search(htable_t *ht, void *key, void *item);
extern int htable_get_probe_cnt(htable_t *ht, void *key);
extern int htable_remove(htable_t *ht, void *key, int *relocs);
extern bool htable_check_validity(htable_t *ht);
