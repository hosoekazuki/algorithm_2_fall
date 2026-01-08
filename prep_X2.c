#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define L 15
#define Q 3
#define GSIZE 1000   // 10^3
#define MAXN 1000000

typedef struct {
    uint32_t *id;
    uint32_t sz, cap;
} Posting;

Posting index_tbl[GSIZE];

/* 3-gram を 0..999 に変換 */
static inline int gram_id(const char *s) {
    return (s[0]-'A')*100 + (s[1]-'A')*10 + (s[2]-'A');
}

void push(int g, uint32_t id) {
    Posting *p = &index_tbl[g];
    if (p->sz == p->cap) {
        p->cap = p->cap ? p->cap*2 : 4;
        p->id = realloc(p->id, p->cap * sizeof(uint32_t));
    }
    p->id[p->sz++] = id;
}

int main(int argc, char **argv) {
    FILE *db = fopen(argv[1], "r");
    FILE *out = fopen("index.bin", "wb");
    char s[32];
    uint32_t id = 0;

    while (fgets(s, sizeof(s), db)) {
        for (int i = 0; i <= L-Q; i++) {
            int g = gram_id(s+i);
            push(g, id);
        }
        id++;
    }

    /* 索引を保存 */
    for (int g = 0; g < GSIZE; g++) {
        fwrite(&index_tbl[g].sz, sizeof(uint32_t), 1, out);
        fwrite(index_tbl[g].id, sizeof(uint32_t),
               index_tbl[g].sz, out);
    }

    return 0;
}
探索　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define L 15
#define Q 3
#define GSIZE 1000
#define TH 4        // 共通3-gramの下限
#define MAXN 1000000

uint32_t *index_tbl[GSIZE];
uint32_t index_sz[GSIZE];

uint8_t cnt[MAXN];
uint32_t touched[50000];
int tcnt;

static inline int gram_id(const char *s) {
    return (s[0]-'A')*100 + (s[1]-'A')*10 + (s[2]-'A');
}

/* 編集距離（3を超えたら打ち切り） */
int edit_dist(const char *a, const char *b) {
    int dp[L+1][L+1];
    for (int i=0;i<=L;i++) dp[i][0]=i;
    for (int j=0;j<=L;j++) dp[0][j]=j;

    for (int i=1;i<=L;i++) {
        int minv = 100;
        for (int j=1;j<=L;j++) {
            int c = (a[i-1]==b[j-1])?0:1;
            dp[i][j] = dp[i-1][j]+1;
            if (dp[i][j-1]+1 < dp[i][j]) dp[i][j]=dp[i][j-1]+1;
            if (dp[i-1][j-1]+c < dp[i][j]) dp[i][j]=dp[i-1][j-1]+c;
            if (dp[i][j] < minv) minv = dp[i][j];
        }
        if (minv > 3) return 4;
    }
    return dp[L][L];
}

int main(int argc, char **argv) {
    FILE *idx = fopen("index.bin", "rb");
    FILE *db  = fopen(argv[1], "r");
    FILE *qr  = fopen(argv[2], "r");

    /* 索引読み込み */
    for (int g=0; g<GSIZE; g++) {
        fread(&index_sz[g], sizeof(uint32_t), 1, idx);
        index_tbl[g] = malloc(index_sz[g] * sizeof(uint32_t));
        fread(index_tbl[g], sizeof(uint32_t), index_sz[g], idx);
    }

    char q[32], b[32];

    while (fgets(q, sizeof(q), qr)) {
        tcnt = 0;

        /* スライド3-gram */
        for (int i=0;i<=L-Q;i++) {
            int g = gram_id(q+i);
            for (uint32_t k=0;k<index_sz[g];k++) {
                uint32_t id = index_tbl[g][k];
                if (cnt[id]++ == 0)
                    touched[tcnt++] = id;
            }
        }

        int found = 0;
        for (int i=0;i<tcnt;i++) {
            uint32_t id = touched[i];
            if (cnt[id] >= TH) {
                fseek(db, id*(L+1), SEEK_SET);
                fgets(b, sizeof(b), db);
                if (edit_dist(q, b) <= 3) {
                    found = 1;
                    break;
                }
            }
        }

        for (int i=0;i<tcnt;i++) cnt[touched[i]] = 0;
        printf("%d", found);
    }
    printf("\n");
    return 0;
}