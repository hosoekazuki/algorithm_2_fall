#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 256
#define MAX_DB_SIZE 200000
#define Q 2                 // Q-gram (Bigram)
#define K 3                 // 許容編集距離

// Q-gram構造体
typedef struct {
    char str[Q + 1];
} QGram;

// 最小値マクロ
#define MIN(a, b, c) (((a) < (b)) ? (((a) < (c)) ? (a) : (c)) : (((b) < (c)) ? (b) : (c)))

// ---------------------------------------------------------
// 編集距離計算 (Levenshtein Distance)
// ---------------------------------------------------------
int levenshtein_distance(const char *s1, const char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    
    if (abs(len1 - len2) > K) return K + 1;

    int *v0 = (int *)malloc((len2 + 1) * sizeof(int));
    int *v1 = (int *)malloc((len2 + 1) * sizeof(int));

    for (int i = 0; i <= len2; i++) v0[i] = i;

    for (int i = 0; i < len1; i++) {
        v1[0] = i + 1;
        int min_dist = v1[0];

        for (int j = 0; j < len2; j++) {
            int cost = (s1[i] == s2[j]) ? 0 : 1;
            v1[j + 1] = MIN(v1[j] + 1, v0[j + 1] + 1, v0[j] + cost);
            if (v1[j + 1] < min_dist) min_dist = v1[j + 1];
        }

        if (min_dist > K) {
            free(v0); free(v1);
            return K + 1;
        }

        for (int j = 0; j <= len2; j++) v0[j] = v1[j];
    }

    int result = v0[len2];
    free(v0);
    free(v1);
    return result;
}

// ---------------------------------------------------------
// Q-gram 関連関数
// ---------------------------------------------------------
int compare_qgrams(const void *a, const void *b) {
    return strncmp(((QGram *)a)->str, ((QGram *)b)->str, Q);
}

int generate_sorted_qgrams(const char *s, QGram **out_qgrams) {
    int len = strlen(s);
    if (len < Q) {
        *out_qgrams = NULL;
        return 0;
    }
    
    int num_qgrams = len - Q + 1;
    *out_qgrams = (QGram *)malloc(num_qgrams * sizeof(QGram));
    
    for (int i = 0; i < num_qgrams; i++) {
        strncpy((*out_qgrams)[i].str, s + i, Q);
        (*out_qgrams)[i].str[Q] = '\0';
    }
    
    qsort(*out_qgrams, num_qgrams, sizeof(QGram), compare_qgrams);
    return num_qgrams;
}

int count_common_qgrams(QGram *q1, int n1, QGram *q2, int n2) {
    int count = 0;
    int i = 0, j = 0;
    while (i < n1 && j < n2) {
        int cmp = strncmp(q1[i].str, q2[j].str, Q);
        if (cmp == 0) {
            count++; i++; j++;
        } else if (cmp < 0) {
            i++;
        } else {
            j++;
        }
    }
    return count;
}

// ---------------------------------------------------------
// ファイル読み込み
// ---------------------------------------------------------
int load_file(const char *filename, char ***lines) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return -1;
    }

    *lines = (char **)malloc(MAX_DB_SIZE * sizeof(char *));
    int count = 0;
    char buffer[MAX_LINE_LEN];

    while (fgets(buffer, MAX_LINE_LEN, fp)) {
        buffer[strcspn(buffer, "\r\n")] = 0; // 改行削除
        if (strlen(buffer) == 0) continue;

        // データは1行=1レコード（プレーンテキスト）として扱う
        // 不要な壊れたパースを排し、そのまま複製する
        // strdup 依存を避けるために手動複製
        size_t n = strlen(buffer) + 1;
        char *copy = (char *)malloc(n);
        if (!copy) {
            fprintf(stderr, "Error: Out of memory while reading %s\n", filename);
            fclose(fp);
            return -1;
        }
        memcpy(copy, buffer, n);
        (*lines)[count] = copy;
        
        count++;
        if (count >= MAX_DB_SIZE) break;
    }

    fclose(fp);
    return count;
}

// ---------------------------------------------------------
// メイン処理
// ---------------------------------------------------------
int main(int argc, char *argv[]) {
    // 引数チェック
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <db_file> <query_file> <output_file>\n", argv[0]);
        return 1;
    }

    const char *db_filename = argv[1];
    const char *query_filename = argv[2];
    const char *output_filename = argv[3];

    // ファイル読み込み
    char **db_lines = NULL;
    char **query_lines = NULL;

    printf("Loading DB from '%s'...\n", db_filename);
    int db_size = load_file(db_filename, &db_lines);
    if (db_size < 0) return 1;

    printf("Loading Queries from '%s'...\n", query_filename);
    int query_size = load_file(query_filename, &query_lines);
    if (query_size < 0) return 1;

    // 出力ファイルオープン
    FILE *fp_out = fopen(output_filename, "w");
    if (!fp_out) {
        fprintf(stderr, "Error: Cannot create output file %s\n", output_filename);
        return 1;
    }

    printf("Processing %d queries against %d DB records...\n", query_size, db_size);

    // 検索ループ
    for (int q_idx = 0; q_idx < query_size; q_idx++) {
        char *query = query_lines[q_idx];
        int q_len = strlen(query);
        
        QGram *query_grams;
        int q_gram_count = generate_sorted_qgrams(query, &query_grams);

        int threshold = q_gram_count - (K * Q);
        if (threshold < 0) threshold = 0;

        int found = 0; // 0: なし, 1: あり

        for (int d_idx = 0; d_idx < db_size; d_idx++) {
            char *target = db_lines[d_idx];
            int t_len = strlen(target);

            // 長さフィルタ
            if (abs(q_len - t_len) > K) continue;

            // Q-gramフィルタ
            QGram *target_grams;
            int t_gram_count = generate_sorted_qgrams(target, &target_grams);
            int common = count_common_qgrams(query_grams, q_gram_count, target_grams, t_gram_count);
            free(target_grams);

            if (common >= threshold) {
                // 編集距離検証
                if (levenshtein_distance(query, target) <= K) {
                    found = 1;
                    break; // 一つでも見つかればループを抜ける
                }
            }
        }

        // 結果をファイルに出力
        fprintf(fp_out, "%d\n", found);
        
        free(query_grams);
        
        // 進捗表示
        if ((q_idx + 1) % 100 == 0) {
            printf("\rProcessed: %d / %d", q_idx + 1, query_size);
            fflush(stdout);
        }
    }

    printf("\nDone. Results saved to '%s'.\n", output_filename);

    // 終了処理
    fclose(fp_out);
    for(int i=0; i<db_size; i++) free(db_lines[i]);
    free(db_lines);
    for(int i=0; i<query_size; i++) free(query_lines[i]);
    free(query_lines);

    return 0;
}