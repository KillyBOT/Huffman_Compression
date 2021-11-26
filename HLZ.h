#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define FREQ_TABLE_SIZE 257 //The 256th element is the EOF signifier
#define TYPE_DATA 0
#define TYPE_FORK 1
#define TYPE_EOF 2

typedef unsigned char byte_t;
typedef unsigned long freq_t;

typedef struct hnode_t{
    unsigned long freq; 
    byte_t type;
    byte_t data;
    struct hnode_t* left;
    struct hnode_t* right;
} hnode_t;

hnode_t* create_hnode_leaf(byte_t data, freq_t freq);
hnode_t* create_hnode_fork(hnode_t* left, hnode_t* right);
void free_hnode(hnode_t* h);
void free_hnodes(hnode_t** h);
void print_hnode(hnode_t* h);
void print_hnodes(hnode_t** h);

hnode_t** init_hdnodes(int f);
hnode_t* build_htree(hnode_t** freqTable);
