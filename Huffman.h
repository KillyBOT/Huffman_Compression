#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define FREQ_TABLE_SIZE 257 //The 256th element is the EOF signifier
#define TYPE_DATA 0
#define TYPE_FORK 1
#define TYPE_EOF 2

#define FLAG_KEEPWRITING 0
#define FLAG_STOPWRITING 1 //Basically, once we're done, just write the final bit to the 

typedef unsigned char byte_t;
typedef unsigned long freq_t;

typedef struct hnode_t{
    unsigned long freq; 
    byte_t type;
    byte_t data;
    struct hnode_t* left;
    struct hnode_t* right;
} hnode_t;

typedef struct path_t{
    unsigned int data;
    int len;
} path_t;

hnode_t* create_hnode_leaf(byte_t data, freq_t freq);
hnode_t* create_hnode_fork(hnode_t* left, hnode_t* right);
hnode_t* create_hnode_from_file(int f);
void free_hnode(hnode_t* h);
void free_hnodes(hnode_t** h);
void print_hnode(hnode_t* h);
void print_hnodes(hnode_t** h);

hnode_t** init_hdnodes(int f);
hnode_t* build_htree(hnode_t** t);

path_t* init_path_table();
void fill_path_table(hnode_t* h, path_t* p, unsigned int pos, int len);

void write_compressed_file(int in, int out, hnode_t* h, path_t* p);
void decompress_file(int in, int out);
void write_tree(int f, hnode_t* h);

void write_bit(int f, unsigned int b, int flag);
void write_bits(int f, unsigned int b, int len, int flag);
int read_bit(int f);
void unsigned_int_print_bits(unsigned int i);
