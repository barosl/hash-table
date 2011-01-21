#include "htable.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "clock.h"

#define HASH_TABLE_SIZE 1070000

typedef struct {
	char name[61];
	double x, y;
	char addr[100];
} rec_t;

void disk_keyer(void *item, void *key) {
	int i;

	rec_t *rec = (rec_t*)item;

	/* normalize the key */
	int name_len = strlen(rec->name);
	memcpy(key, rec->name, name_len);
	for (i=name_len;i<sizeof(rec->name);i++) ((char*)key)[i] = 0;
}

int disk_hasher(void *key, int size) {
	int sum = 0;

	unsigned char *buf = (unsigned char*)key;
	while (*buf && (sum += (*buf)*(*buf)*(*buf), buf++, 1));

	return (sum % size + size) % size;
}

void disk_getter(void *user_data, int pos, void *item) {
	FILE *fp = (FILE*)user_data;
	fseek(fp, sizeof(rec_t)*pos, SEEK_SET);
	int ret = fread(item, sizeof(rec_t), 1, fp);
	(void)ret;
}

void disk_setter(void *user_data, int pos, void *item) {
	FILE *fp = (FILE*)user_data;
	fseek(fp, sizeof(rec_t)*pos, SEEK_SET);
	fwrite(item, sizeof(rec_t), 1, fp);
}

void strip(char *buf) {
	char *buf_pos = buf + strlen(buf);
	while (--buf_pos >= buf && (*buf_pos == '\r' || *buf_pos == '\n')) *buf_pos = '\0';
}

int input(htable_t *ht) {
	char buf[1024];

	printf("* Creating hash table...\n");

	int st = clock_msec();

	FILE *fin = fopen("../names_uniq.txt", "r");
	if (!fin) {
		perror("cannot open input file");
		return -1;
	}

	rec_t rec;

	int i = 0;
	while (fgets(buf, sizeof(buf), fin)) {
		strip(buf);

		snprintf(rec.name, sizeof(rec.name), "%s", buf);
		rec.x = rec.y = ++i;
		*rec.addr = '\0';

		if (htable_add(ht, &rec)) fprintf(stderr, "Duplicate found: %s\n", rec.name);
	}

	fclose(fin);

	int et = clock_msec() - st;

	printf("* Hash table created. (Elapsed time: %d.%03ds)\n", et/1000, et%1000);

	return 0;
}

void process(htable_t *ht) {
	int i;
	char buf[1024], cmd[1024], param[1024];
	rec_t rec;
	char key[sizeof(rec.name)];

	clock_init();

	while (fgets(buf, sizeof(buf), stdin)) {
		strip(buf);

		int arg_cnt = sscanf(buf, "%s %[^\n]", cmd, param);

		if (arg_cnt >= 2 && !strcmp(cmd, "r")) {
			int st = clock_msec();

			ht->keyer(param, key);
			int res = htable_search(ht, key, &rec);

			int et = clock_msec() - st;

			if (res) fprintf(stderr, "Record not found. (Elapsed time: %d.%03ds)\n", et/1000, et%1000);
			else printf("%s %.0f %.0f (Elapsed time: %d.%03ds)\n", rec.name, rec.x, rec.y, et/1000, et%1000);

		} else if (arg_cnt >= 2 && !strcmp(cmd, "f")) {
			int st = clock_msec();

			for (i=0;i<ht->size;i++) {
				if (ht->occupied[i]) {
					ht->getter(ht->user_data, i, &rec);
					if (!strcmp(rec.name, param)) break;
				}
			}

			int et = clock_msec() - st;

			if (i == ht->size) fprintf(stderr, "Record not found. (Elapsed time: %d.%03ds)\n", et/1000, et%1000);
			else printf("%s %.0f %.0f (Elapsed time: %d.%03ds)\n", rec.name, rec.x, rec.y, et/1000, et%1000);

		} else if (arg_cnt >= 1 && !strcmp(cmd, "p")) {
			int sum = 0;
			int cnt = 0;

			for (i=0;i<ht->size;i++) {
				if (ht->occupied[i]) {
					ht->getter(ht->user_data, i, &rec);
					ht->keyer(&rec, key);
					sum += htable_get_probe_cnt(ht, key);
					cnt++;
				}
			}

			printf("Average probe count: %.2f\n", (double)sum/cnt);

		} else if (arg_cnt >= 1 && !strcmp(cmd, "q")) {
			break;

		} else if (arg_cnt >= 2 && !strcmp(cmd, "d")) {
			int st = clock_msec();

			ht->keyer(param, key);
			int relocs;
			int res = htable_remove(ht, key, &relocs);

			int et = clock_msec() - st;

			if (res) fprintf(stderr, "Record not found. (Elapsed time: %d.%03ds)\n", et/1000, et%1000);
			else printf("Relocation count: %d (Elapsed time: %d.%03ds)\n", relocs, et/1000, et%1000);

		} else {
			fprintf(stderr, "Invalid format.\n");
		}
	}
}

int main(void) {
	rec_t rec;

	FILE *fp = fopen("../output.bin", "w+b");

	htable_t *ht = htable_create(HASH_TABLE_SIZE, sizeof(rec_t), sizeof(rec.name), fp, disk_keyer, disk_hasher, disk_getter, disk_setter);

	if (input(ht)) return -1;

	process(ht);

	htable_delete(ht);

	fclose(fp);

	return 0;
}
