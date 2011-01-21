#include "htable.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

htable_t *htable_create(int size, int item_size, int key_size, void *user_data, void (*keyer)(void *item, void *key), int (*hasher)(void *key, int size), void (*getter)(void *user_data, int pos, void *item), void (*setter)(void *user_data, int pos, void *item)) {
	int i;

	htable_t *ht = (htable_t*)malloc(sizeof(htable_t));

	ht->size = size;
	ht->item_size = item_size;
	ht->key_size = key_size;
	ht->user_data = user_data;

	ht->links = (int*)malloc(sizeof(int)*size);
	for (i=0;i<size;i++) ht->links[i] = -1;

	ht->f_links = (int*)malloc(sizeof(int)*size);
	for (i=0;i<size;i++) ht->f_links[i] = i+1;
	ht->f_links[size-1] = -1;

	ht->b_links = (int*)malloc(sizeof(int)*size);
	for (i=0;i<size;i++) ht->b_links[i] = i-1;

	ht->occupied = (bool*)malloc(sizeof(bool)*size);
	for (i=0;i<size;i++) ht->occupied[i] = false;

	ht->cur_f = 0;
	ht->cur_b = size-1;

	ht->keyer = keyer;
	ht->hasher = hasher;
	ht->getter = getter;
	ht->setter = setter;

	return ht;
}

void htable_delete(htable_t *ht) {
	free(ht->links);
	free(ht->f_links);
	free(ht->b_links);
	free(ht->occupied);
	free(ht);
}

static int htable_set_pos_occupied(htable_t *ht, int pos);
static int htable_get_empty_pos(htable_t *ht);
static int htable_get_final_pos(htable_t *ht, int pos);

int htable_add(htable_t *ht, void *item) {
	void *key = malloc(ht->key_size);
	ht->keyer(item, key);

	if (!htable_search(ht, key, NULL)) return -1;

	int pos = ht->hasher(key, ht->size);
	if (!ht->occupied[pos]) {
		htable_set_pos_occupied(ht, pos);
		ht->setter(ht->user_data, pos, item);
		ht->occupied[pos] = true;
	} else {
		int new_pos = htable_get_empty_pos(ht);
		if (new_pos == -1) return -1;
		ht->setter(ht->user_data, new_pos, item);
		ht->occupied[new_pos] = true;

		int final_pos = htable_get_final_pos(ht, pos);
		assert(final_pos != -1);
		ht->links[final_pos] = new_pos;
	}

	return 0;
}

static int htable_get_empty_pos(htable_t *ht) {
	if (ht->cur_f == -1) return -1;
	else return htable_set_pos_occupied(ht, ht->cur_f);
}

static int htable_set_pos_occupied(htable_t *ht, int pos) {
	if (pos != ht->cur_f && pos != ht->cur_b) {
		ht->f_links[ht->b_links[pos]] = ht->f_links[pos];
		ht->b_links[ht->f_links[pos]] = ht->b_links[pos];
	}

	if (pos == ht->cur_f) {
		ht->cur_f = ht->f_links[pos];
		ht->b_links[ht->cur_f] = -1;
	}

	if (pos == ht->cur_b) {
		ht->cur_b = ht->b_links[pos];
		ht->b_links[ht->cur_f] = -1;
	}

	ht->f_links[pos] = ht->b_links[pos] = -1;
	return pos;
}

void htable_print_table(htable_t *ht) {
	int i;

	void *item = malloc(ht->item_size);
	void *item_key = malloc(ht->key_size);

	for (i=0;i<ht->size;i++) printf("%2d ", i);
	printf("\n");

	for (i=0;i<ht->size;i++) {
		if (ht->occupied[i]) {
			ht->getter(ht->user_data, i, item);
			ht->keyer(item, item_key);
			printf("%2d ", *(int*)item_key);
		} else {
			printf(".  ");
		}
	}
	printf("\n");

	printf("----\n");

	free(item);
	free(item_key);
}

int htable_search(htable_t *ht, void *key, void *item) {
	bool del_item = false;
	if (!item) {
		del_item = true;
		item = malloc(ht->item_size);
	}

	void *item_key = malloc(ht->key_size);

	int pos = ht->hasher(key, ht->size);
	if (ht->occupied[pos]) {
		while (1) {
			ht->getter(ht->user_data, pos, item);
			ht->keyer(item, item_key);
			if (!memcmp(item_key, key, ht->key_size)) {
				if (del_item) free(item);
				free(item_key);
				return 0;
			}

			pos = ht->links[pos];
			if (pos == -1) break;
		}
	}

	if (del_item) free(item);
	free(item_key);
	return -1;
}

int htable_get_probe_cnt(htable_t *ht, void *key) {
	int cnt = 0;

	void *item = malloc(ht->item_size);
	void *item_key = malloc(ht->key_size);

	int pos = ht->hasher(key, ht->size);
	if (ht->occupied[pos]) {
		while (1) {
			cnt++;

			ht->getter(ht->user_data, pos, item);
			ht->keyer(item, item_key);
			if (!memcmp(item_key, key, ht->key_size)) {
				free(item);
				free(item_key);
				return cnt;
			}

			pos = ht->links[pos];
			if (pos == -1) break;
		}
	}

	free(item);
	free(item_key);
	return -1;
}

static int htable_get_final_pos(htable_t *ht, int pos) {
	while (1) {
		int new_pos = ht->links[pos];
		if (new_pos == -1) return pos;
		pos = new_pos;
	}
}

static int htable_remove_first(htable_t *ht, int pos);
static int htable_remove_mid(htable_t *ht, int pos, int prev_pos);

int htable_remove(htable_t *ht, void *key, int *relocs) {
	void *item = malloc(ht->item_size);
	void *item_key = malloc(ht->key_size);

	int pos = ht->hasher(key, ht->size);
	if (ht->occupied[pos]) {
		ht->getter(ht->user_data, pos, item);
		ht->keyer(item, item_key);
		if (!memcmp(item_key, key, ht->key_size)) {
			int _relocs = htable_remove_first(ht, pos);
			if (relocs) *relocs = _relocs;

			free(item);
			free(item_key);
			return 0;
		}
	}

	int new_pos;

	while (1) {
		new_pos = ht->links[pos];
		if (new_pos == -1) {
			free(item);
			free(item_key);
			return -1;
		}
		ht->getter(ht->user_data, new_pos, item);
		ht->keyer(item, item_key);
		if (!memcmp(item_key, key, ht->key_size)) break;
		pos = new_pos;
	}

	int _relocs = htable_remove_mid(ht, new_pos, pos);
	if (relocs) *relocs = _relocs;

	free(item);
	free(item_key);
	return 0;
}

static void htable_find_pos(htable_t *ht, int pos, bool (*cond)(void *user_data, int pos), void *user_data, int *found_pos, int *found_prev_pos);
static void htable_set_pos_unoccupied(htable_t *ht, int pos);

static inline bool is_same(void *user_data, int val) { return (int)(intptr_t)user_data == val; }

static int htable_remove_first(htable_t *ht, int pos) {
	int relocs = 0;

	int found_pos, found_prev_pos;
	htable_find_pos(ht, pos, is_same, (void*)(intptr_t)pos, &found_pos, &found_prev_pos);

	void *item = malloc(ht->item_size);

	if (found_pos == -1) {
		ht->occupied[pos] = false;
		ht->links[pos] = -1;

		htable_set_pos_unoccupied(ht, pos);
	} else {
		ht->getter(ht->user_data, found_pos, item);
		ht->setter(ht->user_data, pos, item);
		relocs++;

		relocs += htable_remove_mid(ht, found_pos, found_prev_pos);
	}

	free(item);

	return relocs;
}

typedef struct {
	int poses[100]; /* FIXME */
	int pos_cnt;
} pos_set_t;

static inline bool is_not_in(void *user_data, int val) {
	int i;

	pos_set_t *pos_set = (pos_set_t*)user_data;

	for (i=0;i<pos_set->pos_cnt;i++) {
		if (pos_set->poses[i] == val) return false;
	}

	return true;
}

static int htable_remove_mid(htable_t *ht, int pos, int prev_pos) {
	int relocs = 0;

	int found_pos, found_prev_pos;
	htable_find_pos(ht, pos, is_same, (void*)(intptr_t)pos, &found_pos, &found_prev_pos);

	if (found_pos == -1) {
		ht->links[prev_pos] = ht->links[pos];

		ht->occupied[pos] = false;
		ht->links[pos] = -1;

		htable_set_pos_unoccupied(ht, pos);
	} else {
		pos_set_t pos_set;
		pos_set.pos_cnt = 0;

		int cur_pos = pos;
		while (1) {
			pos_set.poses[pos_set.pos_cnt++] = cur_pos;
			cur_pos = ht->links[cur_pos];
			if (cur_pos == -1) break;
		}

		int found_pos, found_prev_pos;
		htable_find_pos(ht, pos, is_not_in, &pos_set, &found_pos, &found_prev_pos);

		void *item = malloc(ht->item_size);

		if (found_pos == -1) {
			ht->links[prev_pos] = -1;
			relocs += htable_remove_first(ht, pos);
		} else {
			ht->getter(ht->user_data, found_pos, item);
			ht->setter(ht->user_data, pos, item);
			relocs++;

			relocs += htable_remove_mid(ht, found_pos, found_prev_pos);
		}

		free(item);
	}

	return relocs;
}

static void htable_find_pos(htable_t *ht, int pos, bool (*cond)(void *user_data, int pos), void *user_data, int *found_pos, int *found_prev_pos) {
	*found_pos = -1, *found_prev_pos = -1;

	void *item = malloc(ht->item_size);
	void *item_key = malloc(ht->key_size);

	while (1) {
		int new_pos = ht->links[pos];
		if (new_pos == -1) break;
		ht->getter(ht->user_data, new_pos, item);
		ht->keyer(item, item_key);
		if (cond(user_data, ht->hasher(item_key, ht->size))) {
			*found_pos = new_pos;
			*found_prev_pos = pos;
		}
		pos = new_pos;
	}

	free(item);
	free(item_key);
}

bool htable_check_validity(htable_t *ht) {
	int i;

	void *item = malloc(ht->item_size);
	void *item_key = malloc(ht->key_size);

	for (i=0;i<ht->size;i++) {
		if (ht->occupied[i]) {
			ht->getter(ht->user_data, i, item);
			ht->keyer(item, item_key);
			if (htable_search(ht, item_key, NULL)) {
				free(item);
				free(item_key);
				return false;
			}
		}
	}

	free(item);
	free(item_key);
	return true;
}

static void htable_set_pos_unoccupied(htable_t *ht, int pos) {
	int prev_f = ht->cur_f;

	ht->f_links[pos] = ht->cur_f;
	ht->cur_f = pos;

	ht->b_links[prev_f] = pos;
}
