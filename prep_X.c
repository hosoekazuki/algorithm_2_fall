/* prep_X.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define MAX_N 1000000
#define L 15
#define GRAM 3
#define ALPHA 10
#define GSIZE 1000   // 10^3

typedef struct {
    uint32_t *ids;
    uint32_t size;
    uint32_t cap;
} Posting;

Posting index_tbl[GSIZE];

static inline int gram_id(const char *s) {
    return (s[0]-'A')*100 + (s[1]-'A')*10 + (s[2]-'A');
}

void push_posting(int g, uint32_t id) {
    Posting *p = &index_tbl[g];
    if (p->size == p->cap) {
        p->cap = p->cap ? p->cap*2 : 4;
        p->ids = realloc(p->ids, p->cap * sizeof(uint32_t));
    }
    p->ids[p->size++] = id;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <db_file>\n", argv[0]);
        return 1;
    }

    clock_t start = clock();

    FILE *db = fopen(argv[1], "r");
    if (!db) {
        perror("fopen db");
        return 1;
    }
    char buf[32];
    uint32_t id = 0;

    while (fgets(buf, sizeof(buf), db)) {
        for (int i = 0; i <= L-GRAM; i++) {
            int g = gram_id(buf+i);
            push_posting(g, id);
        }
        id++;
    }

    /* 書き出し: 標準出力(STDOUT)に索引をバイナリで出力 */
    for (int g = 0; g < GSIZE; g++) {
        fwrite(&index_tbl[g].size, sizeof(uint32_t), 1, stdout);
        fwrite(index_tbl[g].ids, sizeof(uint32_t), index_tbl[g].size, stdout);
    }

    fclose(db);
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    fprintf(stderr, "time required %.3f sec\n", elapsed);
    return 0;
}
 
