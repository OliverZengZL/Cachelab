#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "cachelab.h"

typedef struct cache_lines
{
    int valid, lru_counter;
    unsigned long long tag;
} cl;

int main(int argc, char **argv)
{
    int opt, s, E, b;
    int set_mask;
    char trc[20] = {0};
    FILE *pTrace;

    while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1)
    {
        switch (opt)
        {
        case 's':
            s = atoi(optarg);
            set_mask = ~(~0 << s);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            strcpy(trc, optarg);
            pTrace = fopen(trc, "r+");
            break;
        default:
            break;
        }
    }
    
    // cache[1 << s][E]
    cl **cache;
    cache = (cl **)malloc(sizeof(cl *) * (1 << s));
    for (int i = 0; i < 1 << s; ++i)
    {
        cache[i] = (cl *)malloc(sizeof(cl) * E);
    }

    char id;
    unsigned long long addr;
    int sz;

    int hits = 0, misses = 0, evictions = 0;

    while (fscanf(pTrace, " %c %llx,%d", &id, &addr, &sz) > 0)
    {
        if (id == 'I' || id == ' ') // 'I' Instruction load
            continue;
        else
        {
            printf("%c %llx,%d ", id, addr, sz);
            int set_num = (addr >> b) & set_mask;
            unsigned long long tag_num = addr >> (b + s);
            int empty_index = 0;
            int hit_flag = 0;
            int largest_cnt = -1;
            int largest_index = 0;
            for (int i = 0; i < E; ++i)
            {
                if (cache[set_num][i].valid) //current line is valid
                {
                    if (cache[set_num][i].tag == tag_num) //hit
                    {
                        hits++;
                        printf("hit ");
                        if (id == 'M')
                        {
                            hits++;
                            printf("hit ");
                        }
                        hit_flag = 1;
                        cache[set_num][i].lru_counter = -1;
                    }
                    empty_index++;
                    cache[set_num][i].lru_counter++;

                    if (largest_cnt < cache[set_num][i].lru_counter) //lru
                    {
                        largest_cnt = cache[set_num][i].lru_counter;
                        largest_index = i;
                    }
                }

                // miss
                else if (!hit_flag)
                {
                    cache[set_num][empty_index].valid = 1;
                    cache[set_num][empty_index].lru_counter = 0;
                    cache[set_num][empty_index].tag = tag_num;
                    misses++;
                    printf("miss ");
                    if (id == 'M')
                    {
                        hits++;
                        printf("hit ");
                    }
                    hit_flag = 1;
                    break;
                }
            }

            //eviction
            if (!hit_flag && empty_index == E)
            {
                misses++;
                printf("eviction ");
                cache[set_num][largest_index].tag = tag_num;
                cache[set_num][largest_index].lru_counter = 0;
                evictions++;
                if (id == 'M')
                {
                    hits++;
                    printf("hit ");
                }
            }
        }
        printf("\n");
    }

    for (int i = 0; i < 1 << s; ++i)
    {
        free(cache[i]);
    }
    free(cache);
    fclose(pTrace);

    printSummary(hits, misses, evictions);
    return 0;
}
