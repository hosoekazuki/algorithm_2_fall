#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_N 1000000
#define GRAM 3
#define GSIZE 1000
#define THRESH 4 

uint32_t *index_tbl[GSIZE];
uint32_t index_sz[GSIZE];
uint8_t counter[MAX_N];

static inline int gram_id(const char *s) {
    for (int i = 0; i < GRAM; i++) {
        if (s[i] < 'A' || s[i] > 'J') return -1;
    }
    return (s[0]-'A')*100 + (s[1]-'A')*10 + (s[2]-'A');
}

int main(int argc, char *argv[]) {
    if (argc < 3) return 1;

    // 索引ファイルの読み込み (argv[2])
    FILE *idx = fopen(argv[2], "rb");
    if (!idx) return 1;
    
    uint32_t max_id = 0;
    for (int g = 0; g < GSIZE; g++) {
        if (fread(&index_sz[g], sizeof(uint32_t), 1, idx) != 1) break;
        if (index_sz[g] > 0) {
            index_tbl[g] = malloc(index_sz[g] * sizeof(uint32_t));
            fread(index_tbl[g], sizeof(uint32_t), index_sz[g], idx);
            for (uint32_t k = 0; k < index_sz[g]; k++) {
                if (index_tbl[g][k] > max_id) max_id = index_tbl[g][k];
            }
        }
    }
    fclose(idx);
    uint32_t db_size = max_id + 1;

    // クエリデータの読み込み (argv[1])
    FILE *qry = fopen(argv[1], "r");
    if (!qry) return 1;

    char q[64];
    char *out_line = malloc(db_size + 1);

    while (fgets(q, sizeof(q), qry)) {
        q[strcspn(q, "\r\n")] = '\0';
        int len = strlen(q);

        for (int i = 0; i <= len - GRAM; i++) {
            int g = gram_id(q + i);
            if (g != -1) {
                for (uint32_t k = 0; k < index_sz[g]; k++) {
                    counter[index_tbl[g][k]]++;
                }
            }
        }

        // ビットベクトル生成
        for (uint32_t id = 0; id < db_size; id++) {
            out_line[id] = (counter[id] >= THRESH) ? '1' : '0';
            counter[id] = 0; 
        }
        out_line[db_size] = '\0';
        puts(out_line);
    }

    free(out_line);
    fclose(qry);
    return 0;
}