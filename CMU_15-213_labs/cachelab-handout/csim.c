#include "cachelab.h"
#include<stdio.h>
#include<stdbool.h>
#include<getopt.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<string.h>
#include<assert.h>

/*
 * This struct stores info about cache from command-line arguments
 */
typedef struct {
    char *trace_file;
    int set_num;
    int line_num;
    int block_num;
    bool verbose;
}ArgsInfo;
ArgsInfo args_info;

/*
 * This struct stores cache line info
 */
typedef struct {
    bool valid;
    int tag;
    size_t LRU_counter;
}CacheLine;

/*
 * This struct stores cache sets which contains the array of cache lines
 */
typedef struct {
    CacheLine *lines;
}CacheSet;

typedef CacheSet* Cache;

size_t hit, miss, evict;

/*
 * print_help_info - When meeting -h or option errors, print help info 
 *                   to stdout
 */
void print_help_info() {
    printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
    printf("Options:\n");
    printf("-h: Optional help flag that prints usage info\n");
    printf("-v: Optional verbose flag that displays trace info\n");
    printf("-s <s>: Number of set index bits(S = 2^s is the number of sets)\n");
    printf("-E <E>: Associativity (number of lines per set)\n");
    printf("-b <b>: Number of block bits (B = 2 ^ b is the block size)\n");
    printf("-t <tracefile>: Name of the valgrind trace to replay\n");
    exit(0);
}

/*
 * check_option_errors - Check whether options are followed by appropriate 
 *                       number of arguments. We only use it to check options that
 *                       requires an argument.
 */
void check_option_errors(char opt, char *optarg) {
    if(*optarg == '-') {
        printf("Option -%c missed required command-line argument\n", opt);
        print_help_info();
        exit(0);
    }
}

/*
 * parse_args - Parse the command-line options using getopt() 
 *              to get info about cache
 */
void parse_args(int argc, char **argv) {
    extern char *optarg; 
    char opt;
    while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch(opt) {
            case 's':
                check_option_errors(opt, optarg);
                args_info.set_num = atoi(optarg);
                break;
            case 'E':
                check_option_errors(opt, optarg);
                args_info.line_num = atoi(optarg);
                break;
            case 'b':
                check_option_errors(opt, optarg);
                args_info.block_num = atoi(optarg);
                break;
            case 't':
                check_option_errors(opt, optarg);
                args_info.trace_file = optarg;
                break;
            case 'v':
                args_info.verbose = true;
                break;
            case 'h':
                print_help_info();
            default:
                fprintf(stderr, "Type -h for help\n");
                exit(1);
        }
    }
}

/*
 * initialize_cache - Allocate dynamic memory to initialize cache with S and E. 
 *                    If set_num and line_num > 0, return the pointer to the dynamic 
 *                    memory. Otherwise, return NULL.
 */
Cache initialize_cache() {
    if(args_info.set_num <= 0 || args_info.line_num <= 0)
        return NULL;
    int S = pow(2, args_info.set_num);
    int E = args_info.line_num;
    Cache cache = (Cache)calloc(S, sizeof(CacheSet));
    if(!cache) {
        perror("calloc failed");
        exit(EXIT_FAILURE);
    }
    for(int i = 0;i != S;++i) {
        cache[i].lines = (CacheLine*)calloc(E, sizeof(CacheLine));
        if(!cache[i].lines) {
            perror("calloc failed");
            exit(EXIT_FAILURE);
        }
    }
    return cache;
}

/*
 * free_cache - If the pointer isn't NULL, free dynamic memory allocated for cache.
 *              Otherwise, return directly.
 */
void free_cache(Cache cache) {  
    if(!cache)
        return;
    int S = pow(2, args_info.set_num);
    for(int i = 0;i != S;++i) {
        free(cache[i].lines);
    }
    free(cache); 
}

/*
 * is_hit() - Judge whether cache hit or not by checking valid bit and tag.
 *            If cache hit, increment corresponding LRU_counter
 */
bool is_hit(Cache cache, size_t set_index, int addr_tag) {
    for(int i = 0;i != args_info.line_num;++i) {
        if(cache[set_index].lines[i].valid == true && 
                cache[set_index].lines[i].tag == addr_tag) {
            cache[set_index].lines[i].LRU_counter++;
            return true;
        }
    }
    return false;
}

/*
 * load_and store - If cache hit, do nothing other than increment hit counter. 
 *                  If the cache set is not full but miss, increment miss counter
 *                  and update cache. If cache is full and miss, we use the LRU
 *                  replacement policy to choose line to evict.
 */
void load_and_store(Cache cache, size_t set_index, int addr_tag) {
    if(is_hit(cache, set_index, addr_tag)) {
        hit++;
        if(args_info.verbose) 
            printf("hit\n");
    } else {
        miss++;
        for()
    }
}


void test_parse_args(int argc, char **argv) {
    //test -v -s 10 -E 4 -b 2 -t ./test-trace-file.trace
    if(argc == 9 && strcmp(argv[1], "-s") == 0 && strcmp(argv[3], "-E") == 0 
            && strcmp(argv[5],"-b") == 0 && strcmp(argv[7], "-t") == 0) {
        parse_args(argc, argv);
        assert(args_info.verbose == true);
        assert(args_info.set_num == 10);
        assert(args_info.line_num == 4);
        assert(args_info.block_num == 2);
        assert(strcmp(args_info.trace_file, "./test-trace-file.trace") == 0);
    }
}

void test_is_hit() {
    Cache cache = initialize_cache();
    //test not hit
    assert(is_hit(cache, 0, 2) == false);
    cache[0].lines[0].tag = 2;
    assert(is_hit(cache, 0, 2) == false);
    cache[0].lines[0].valid = true;
    cache[0].lines[0].tag = 1;
    assert(is_hit(cache, 0, 2) == false);
    //test hit
    cache[0].lines[0].tag = 2;
    assert(is_hit(cache, 0, 2) == true);
    free_cache(cache);
}

void test_load_and_store() {
    //test miss
    initialize_cache();
    load_and_store(0,0);
    assert(hit == 0 && miss == 1 && evict == 0);
    //test hit
    load_and_store(0,0);
    assert(hit == 1 && miss = 1 && evict == 0);
    //test evict
    for(int i = 1;i <= args_info.line_num;++i) {
        load_and_store(0,i);
    }
    assert(hit == 1 && miss == 3 && evict == 1);
}

int main(int argc, char **argv)
{
    parse_args(argc, argv);
    //test_parse_args(argc,argv);
    //test_is_hit();
    test_load_and_store();
        
    //open trace file
   // FILE *fp = fopen(args_info.trace_file, "r");
   // if(!fp) {
   //     perror("File opening failed");
   //     exit(EXIT_FAILURE);
   // }
    
    //Cache cache = initialize_cache();

   // //parse trace file line by line
   // char opt;
   // size_t address;
   // size_t size;
   // size_t mask = 0;
   // for(int i = 0;i != args_info.set_num;++i) {
   //     mask = (mask << 1) + 0xf;
   // }
   // while(fscanf(fp, "%c %zu,%zu", &opt, &address, &size) == 3) {
   //     if(opt == 'I') 
   //         continue;
   //     if(args_info.verbose)
   //         printf("%c %zu,%zu ", opt, address, size);
   //     size_t set_index = (address >> args_info.block_num) & mask;
   //     int addr_tag = address >> (args_info.block_num + args_info.set_num);
   //     if(opt == 'L' || opt == 'S') {
   //         load_and_store(set_index, addr_tag);
   //     } else {
   //         modify(set_index, addr_tag);
   //     }
   // }
    //printSummary(0, 0, 0);
    return 0;
}
