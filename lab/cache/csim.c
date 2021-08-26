#include "cachelab.h"

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <stdint.h>
#include <assert.h>

int s, E, b, verbose = 0;
int hits = 0, misses = 0, evictions = 0;
FILE *fp = NULL;
char filename[20];

const char *usage = "Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n";
void parseArgument(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
            case 'h':
                fprintf(stdout, usage, argv[0]);
                exit(1);
            case 'v':
                verbose = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                fp = fopen(optarg, "r");
                break;
            default:
                fprintf(stdout, usage, argv[0]);
                exit(1);
        }
    }
}

typedef struct {
    int valid;
    int lru;
    uint64_t tag;
} cacheLine;

typedef cacheLine *cacheSet;
typedef cacheSet *cache;

int lru_cnt = 0;
cache *my_cache = NULL;

int visitCache(uint64_t address) {
    lru_cnt++;
    uint64_t tag = address >> (s + b);
    uint32_t setIndex = address >> b & ((1 << s) - 1);
    int hit = 0, miss = 1, evict = 0;
    for (int i = 0; i < E; i++) {
        if (my_cache[setIndex][i]->valid && my_cache[setIndex][i]->tag == tag) {
            hit = 1;
            my_cache[setIndex][i]->lru = lru_cnt;
            break;
        }
    }
    if (hit) miss = 0;
    if (miss) {
        int ok = 0;
        for (int i = 0; i < E; i++) {
            if (!my_cache[setIndex][i]->valid) {
                my_cache[setIndex][i]->lru = lru_cnt;
                my_cache[setIndex][i]->tag = tag;
                ok = 1;
                break;
            }
        }
        if (!ok) {
            int id = 0, min_lru = 1e9;
            for (int i = 0; i < E; i++) {
                if (my_cache[setIndex][i]->lru <= min_lru) {
                    min_lru = my_cache[setIndex][i]->lru;
                    id = i;
                }
            }
            my_cache[setIndex][id]->lru = lru_cnt;
            my_cache[setIndex][id]->tag = tag;
            evict = 1;
        }
    }
    if (hit) return 0;
    return miss + evict;
}

void simulate() {
    // TODO: malloc
    int B = 1 << b, S = 1 << s;
    uint64_t address;
    int sz, ret;
    char buf[20], op;

    my_cache = malloc(sizeof(cacheSet) * S);
    for (int i = 0; i < S; i++)
        my_cache[i] = (cacheSet) malloc(sizeof(cacheLine) * E);
    for (int i = 0; i < S; i++)
        for (int j = 0; j < E; j++)
            my_cache[i][j]->lru = 0;

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (buf[0] == 'I') continue;
        sscanf(" %c %lx,%d\n", op, &address, &sz);
        switch (op) {
            case 'S':
                ret = visitCache(address);
                break;
            case 'M':
                ret = visitCache(address);
                break;
            case 'L':
                ret = visitCache(address);
                break;
            default:
                break;
        }
        if (verbose) {
            if (ret == 0) {
                printf("%c %lx,%d hit\n",op,address,sz);
            } else if (ret == 1) {
                printf("%c %lx,%d miss\n",op,address,sz);
            } else {
                printf("%c %lx,%d eviction\n",op,address,sz);
            }
        }
    }

    for (int i = 0; i < S; i++)
        free(my_cache[i]);
    free(my_cache);
}

int main(int argc, char **argv) {
    parseArgument(argc, argv);
    simulate();
    printSummary(0, 0, 0);
    return 0;
}
