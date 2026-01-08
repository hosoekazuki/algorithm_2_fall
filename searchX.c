#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define MAX_N 1000000
#define L 15
#define GRAM 3
#define GSIZE 1000
#define THRESH 4   // 共通3-gram数の下限

uint32_t *index_tbl[GSIZE];
uint32_t index_sz[GSIZE];

uint8_t counter[MAX_N];

/* 本仕様では編集距離の検証は行わず、Q-gramフィルタのみでビットベクトルを生成 */

static inline int gram_id(const char *s) {
    return (s[0]-'A')*100 + (s[1]-'A')*10 + (s[2]-'A');
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <query_file> <index_file>\n", argv[0]);
        return 1;
    }

    clock_t start = clock();

    FILE *idx = fopen(argv[2], "rb");
    if (!idx) {
        perror("fopen index");
        return 1;
    }
    FILE *qry = fopen(argv[1], "r");
    if (!qry) {
        perror("fopen query");
        fclose(idx);
        return 1;
    }

    /* 索引読み込み */
    uint32_t max_id = 0;
    for (int g=0; g<GSIZE; g++) {
        fread(&index_sz[g], sizeof(uint32_t), 1, idx);
        index_tbl[g] = malloc(index_sz[g] * sizeof(uint32_t));
        fread(index_tbl[g], sizeof(uint32_t), index_sz[g], idx);
        for (uint32_t k=0; k<index_sz[g]; k++) {
            if (index_tbl[g][k] > max_id) max_id = index_tbl[g][k];
        }
    }
    uint32_t db_size = max_id + 1;

    char q[32];

    while (fgets(q, sizeof(q), qry)) {
        for (int i=0;i<=L-GRAM;i++) {
            int g = gram_id(q+i);
            if (g < 0 || g >= GSIZE) continue;
            for (uint32_t k=0;k<index_sz[g];k++) {
                uint32_t id = index_tbl[g][k];
                counter[id]++;
            }
        }

        for (uint32_t id=0; id<db_size; id++) {
            int bit = (counter[id] >= THRESH) ? 1 : 0;
            putchar(bit ? '1' : '0');
            counter[id] = 0; // reset for next query
        }
        putchar('\n');
    }
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    fprintf(stderr, "time required %.3f sec\n", elapsed);
    return 0;
}