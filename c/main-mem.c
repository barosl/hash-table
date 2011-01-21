#include <string.h>
#include "htable.h"
#include <stdio.h>

#define MAX_NAME_LEN 50

typedef struct {
	int id;
	char name[MAX_NAME_LEN];
} rec_t;

rec_t recs[11];

void mem_keyer(void *item, void *key) {
	*(int*)key = ((rec_t*)item)->id;
}

int mem_hasher(void *key, int size) {
	return *(int*)key % size;
}

void mem_getter(void *user_data, int pos, void *item) {
	rec_t *recs = (rec_t*)user_data;
	memcpy(item, &recs[pos], sizeof(recs[pos]));
}

void mem_setter(void *user_data, int pos, void *item) {
	rec_t *recs = (rec_t*)user_data;
	memcpy(&recs[pos], item, sizeof(recs[pos]));
}

int main(void) {
	int i;
	rec_t rec;
	int key;

	htable_t *ht = htable_create(sizeof(recs)/sizeof(recs[0]), sizeof(rec_t), sizeof(int), recs, mem_keyer, mem_hasher, mem_getter, mem_setter);

	int ids[] = {11, 22, 12, 33, 23, 15, 36, 17, 27, 19};
	char *names[] = {"kim", "lee", "park", "jung", "nam", "ra", "son", "ban", "Shin", "choi"};

	for (i=0;i<sizeof(ids)/sizeof(ids[0]);i++) {
		rec.id = ids[i];
		snprintf(rec.name, sizeof(rec.name), "%s", names[i]);
		htable_add(ht, &rec);
	}

	htable_print_table(ht);

	key = 22;
	htable_remove(ht, &key, NULL);

	htable_print_table(ht);

	htable_delete(ht);

	return 0;
}
