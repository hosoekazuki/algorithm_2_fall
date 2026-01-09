#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define L 15
#define GRAM 3
#define GSIZE 1000 // 10^3 (A-J)

typedef struct {
    uint32_t *ids;
    uint32_t size;
    uint32_t cap;
} Posting;

Posting index_tbl[GSIZE];

static inline int gram_id(const char *s) {
    for (int i = 0; i < GRAM; i++) {
        if (s[i] < 'A' || s[i] > 'J') return -1;
    }
    return (s[0]-'A')*100 + (s[1]-'A')*10 + (s[2]-'A');
}

void push_posting(int g, uint32_t id) {
    if (g < 0 || g >= GSIZE) return;
    Posting *p = &index_tbl[g];
    if (p->size == p->cap) {
        p->cap = p->cap ? p->cap * 2 : 8;
        p->ids = realloc(p->ids, p->cap * sizeof(uint32_t));
    }
    p->ids[p->size++] = id;
}

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;

    FILE *db = fopen(argv[1], "r");
    if (!db) return 1;

    char buf[64];
    uint32_t id = 0;

    while (fgets(buf, sizeof(buf), db)) {
        buf[strcspn(buf, "\r\n")] = '\0';
        int len = strlen(buf);
        for (int i = 0; i <= len - GRAM; i++) {
            int g = gram_id(buf + i);
            if (g != -1) push_posting(g, id);
        }
        id++;
    }

    // 標準出力(STDOUT)に書き出し
    for (int g = 0; g < GSIZE; g++) {
        fwrite(&index_tbl[g].size, sizeof(uint32_t), 1, stdout);
        if (index_tbl[g].size > 0) {
            fwrite(index_tbl[g].ids, sizeof(uint32_t), index_tbl[g].size, stdout);
        }
    }

    fclose(db);
    return 0;
}