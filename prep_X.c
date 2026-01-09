#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define L 15
#define G3 3
#define G4 4
#define G3SIZE 1000
#define G4SIZE 10000
#define MAXN 1000000

typedef struct {
    uint32_t *id;
    uint32_t sz, cap;
} Posting;

Posting idx3[G3SIZE], idx4[G4SIZE];
char db[MAXN][L+1];
uint32_t N = 0;

static inline int gram3(const char *s){
    return (s[0]-'A')*100 + (s[1]-'A')*10 + (s[2]-'A');
}
static inline int gram4(const char *s){
    return (s[0]-'A')*1000 + (s[1]-'A')*100
         + (s[2]-'A')*10   + (s[3]-'A');
}

void push(Posting *p, uint32_t id){
    if(p->sz == p->cap){
        p->cap = p->cap ? p->cap * 2 : 4;
        p->id = realloc(p->id, p->cap * sizeof(uint32_t));
    }
    p->id[p->sz++] = id;
}

int main(int argc, char **argv){
    FILE *fp = fopen(argv[1], "r");
    char s[32];

    while(fgets(s, sizeof(s), fp)){
        s[strcspn(s, "\n")] = '\0';   // ★ 修正点
        if(strlen(s) < L) continue;

        strcpy(db[N], s);

        for(int i=0;i<=L-G3;i++) push(&idx3[gram3(s+i)], N);
        for(int i=0;i<=L-G4;i++) push(&idx4[gram4(s+i)], N);
        N++;
    }

    /* ===== STDOUT ===== */
    fwrite(&N, 4, 1, stdout);

    for(int g=0; g<G3SIZE; g++){
        fwrite(&idx3[g].sz, 4, 1, stdout);
        fwrite(idx3[g].id, 4, idx3[g].sz, stdout);
    }
    for(int g=0; g<G4SIZE; g++){
        fwrite(&idx4[g].sz, 4, 1, stdout);
        fwrite(idx4[g].id, 4, idx4[g].sz, stdout);
    }

    fwrite(db, L+1, N, stdout);
    return 0;
}
