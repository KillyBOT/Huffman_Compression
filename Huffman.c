#include "HLZ.h"

hnode_t* create_hnode_leaf(byte_t data, freq_t freq){
    hnode_t* h = malloc(sizeof(hnode_t));

    h->type = TYPE_DATA;
    h->data = data;
    h->freq = freq;
    h->left = NULL;
    h->right = NULL;

    return h;
}

hnode_t* create_hnode_fork(hnode_t* left, hnode_t* right){

    hnode_t* h = malloc(sizeof(hnode_t));

    h->type = TYPE_FORK;
    h->left = left;
    h->right = right;
    h->freq = left->freq + right->freq;

    return h;
}

void print_hnode(hnode_t* h){
    if(h->left){
        print_hnode(h->left);
    }
    if(h->right){
        print_hnode(h->right);
    }

    if(h->type == TYPE_DATA){
        printf("Data:[%.2X]\tFreq:[%ld]",h->data,h->freq);
    } else if (h->type == TYPE_EOF)
    {
        printf("EOF signifier");
    }
    printf("\n");
}

void print_hnodes(hnode_t** h){
    for(int i = 0; i < FREQ_TABLE_SIZE; i++){
        if(h[i]) print_hnode(h[i]);
    }
}

void free_hnode(hnode_t* h){
    if(h->left) free_hnode(h->left);
    if(h->right) free_hnode(h->right);
    free(h);
}

void free_hnodes(hnode_t** h){
    for(int i = 0; i < FREQ_TABLE_SIZE; i++){
        if(h[i]) free_hnode(h[i]);
    }
    free(h);
}

hnode_t** init_hnodes(int f){ //Parses the file and creates the initial huffman nodes, with corresponding frequencies
    
    hnode_t** h = malloc(sizeof(hnode_t*) * FREQ_TABLE_SIZE);
    byte_t* b = malloc(sizeof(byte_t));

    for(int i = 0; i < FREQ_TABLE_SIZE; i++){
        h[i] = create_hnode_leaf((byte_t)i, 0);
    }
    h[FREQ_TABLE_SIZE-1]->type = TYPE_EOF;

    //Set the cursor to position 0
    lseek(f, 0, SEEK_SET);

    //Find the frequency of each byte in the file
    while(read(f,b,sizeof(byte_t))) h[*b]->freq++;

    //Get rid of any pieces of data with a frequency of 0
    for(int i = 0; i < FREQ_TABLE_SIZE; i++){
        if(h[i]->type == TYPE_DATA && h[i]->freq == 0){
            free_hnode(h[i]);
            h[i] = NULL;
        }
    }

    free(b);

    return h;
}

int main(int argc, char** argv){

    int testFile = open("test.txt",O_RDONLY);
    
    hnode_t** freqTable = init_hnodes(testFile);
    print_hnodes(freqTable);

    free_hnodes(freqTable);

    close(testFile);

    return 0;
}