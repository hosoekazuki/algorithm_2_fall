#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define L 15
#define G3 3
#define G4 4
#define G3SIZE 1000
#define G4SIZE 10000
#define TH3 4
#define MAXN 1000000
#define MAXCAND 60000

uint32_t *idx3[G3SIZE], *idx4[G4SIZE];
uint32_t df3[G3SIZE], df4[G4SIZE];
char db[MAXN][L+1];
uint32_t N;

uint8_t mark[MAXN], cnt3[MAXN];
uint32_t touched[MAXCAND];
int tcnt;

static inline int gram3(const char *s){
    return (s[0]-'A')*100 + (s[1]-'A')*10 + (s[2]-'A');
}
static inline int gram4(const char *s){
    return (s[0]-'A')*1000 + (s[1]-'A')*100
         + (s[2]-'A')*10   + (s[3]-'A');
}

/* df 昇順ソート */
int cmp_df4(const void *a, const void *b){
    uint32_t x = df4[*(int*)a];
    uint32_t y = df4[*(int*)b];
    return (x > y) - (x < y);
}

/* 編集距離 ≤3 */
int edit_distance_le3(const char *a, const char *b){
    int dp[L+1][L+1];
    for(int i=0;i<=L;i++) dp[i][0]=i;
    for(int j=0;j<=L;j++) dp[0][j]=j;

    for(int i=1;i<=L;i++){
        int rmin = 100;
        for(int j=1;j<=L;j++){
            int c = (a[i-1]==b[j-1]) ? 0 : 1;
            int v = dp[i-1][j] + 1;
            if(dp[i][j-1] + 1 < v) v = dp[i][j-1] + 1;
            if(dp[i-1][j-1] + c < v) v = dp[i-1][j-1] + c;
            dp[i][j] = v;
            if(v < rmin) rmin = v;
        }
        if(rmin > 3) return 4;
    }
    return dp[L][L];
}

int main(int argc, char **argv){
    FILE *qf = fopen(argv[1], "r");
    FILE *fp = fopen(argv[2], "rb");

    fread(&N, 4, 1, fp);

    for(int g=0; g<G3SIZE; g++){
        fread(&df3[g], 4, 1, fp);
        idx3[g] = malloc(df3[g] * 4);
        fread(idx3[g], 4, df3[g], fp);
    }
    for(int g=0; g<G4SIZE; g++){
        fread(&df4[g], 4, 1, fp);
        idx4[g] = malloc(df4[g] * 4);
        fread(idx4[g], 4, df4[g], fp);
    }

    fread(db, L+1, N, fp);

    char q[32];
    while(fgets(q, sizeof(q), qf)){
        q[strcspn(q, "\n")] = '\0';   // ★ 修正点
        if(strlen(q) < L){
            putchar('0');
            continue;
        }

        int g3[32], g4[32], n3=0, n4=0;
        for(int i=0;i<=L-G3;i++) g3[n3++] = gram3(q+i);
        for(int i=0;i<=L-G4;i++) g4[n4++] = gram4(q+i);

        qsort(g4, n4, sizeof(int), cmp_df4);

        tcnt = 0;
        for(int i=0;i<n4 && tcnt < MAXCAND;i++){
            int g = g4[i];
            for(uint32_t k=0;k<df4[g];k++){
                uint32_t id = idx4[g][k];
                if(!mark[id]){
                    mark[id]=1;
                    touched[tcnt++] = id;
                    if(tcnt >= MAXCAND) break;
                }
            }
        }

        for(int i=0;i<n3;i++){
            int g = g3[i];
            for(uint32_t k=0;k<df3[g];k++){
                uint32_t id = idx3[g][k];
                if(mark[id]) cnt3[id]++;
            }
        }

        int ok = 0;
        for(int i=0;i<tcnt;i++){
            uint32_t id = touched[i];
            if(cnt3[id] >= TH3 &&
               edit_distance_le3(q, db[id]) <= 3){
                ok = 1;
                break;
            }
        }

        for(int i=0;i<tcnt;i++){
            uint32_t id = touched[i];
            mark[id] = cnt3[id] = 0;
        }

        putchar(ok ? '1' : '0');
    }
    putchar('\n');
    return 0;
}
