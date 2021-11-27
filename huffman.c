#include "huffman.h"

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

hnode_t* create_hnode_from_file(int f){
    hnode_t* h;
    byte_t b;

    h = malloc(sizeof(hnode_t));

    read(f,&b,1);

    h->type = b;

    if(h->type == TYPE_FORK){
        h->left = create_hnode_from_file(f);
        h->right = create_hnode_from_file(f);
    }
    else if(h->type == TYPE_DATA){
        read(f,&b,1);
        h->data = b;
    } else if(h->type != TYPE_EOF){
        fprintf(stderr, "Error reading file: are you sure you're reading a compressed file?\n");
        exit(1);
    }

    return h;
}

void print_hnode(hnode_t* h){
    if(h->type == TYPE_DATA){
        printf("Data:[%.2X]\tFreq:[%ld]",h->data,h->freq);
    } 
    else if(h->type == TYPE_FORK){
        printf("Left:[\n");
        print_hnode(h->left);
        printf("]\nRight:[\n");
        print_hnode(h->right);
        printf("]");
    }
    else if (h->type == TYPE_EOF)
    {
        printf("EOF signifier");
    }
    printf("\n");
}

void print_hnodes(hnode_t** h){
    for(int i = 0; i < FREQ_TABLE_SIZE; i++){
        if(h[i]){
            print_hnode(h[i]);
            printf("\n");
        }
    }
}

void free_hnode(hnode_t* h){
    if(h->type == TYPE_FORK){
        free_hnode(h->left);
        free_hnode(h->right);
    }
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
    byte_t b;

    //Create the node structures that fill the frequency table
    for(int i = 0; i < FREQ_TABLE_SIZE; i++){
        h[i] = create_hnode_leaf((byte_t)i, 0);
    }
    h[FREQ_TABLE_SIZE-1]->type = TYPE_EOF;

    //Set the cursor to position 0
    lseek(f, 0, SEEK_SET);

    //Find the frequency of each byte in the file
    while(read(f,&b,1)) h[b]->freq++;

    //Get rid of any pieces of data with a frequency of 0 (except the EOF signifier)
    for(int i = 0; i < FREQ_TABLE_SIZE-1; i++){
        if(h[i]->freq == 0){
            free_hnode(h[i]);
            h[i] = NULL;
        }
    }

    return h;
}

//Convert the frequency table into an actual huffman tree
hnode_t* build_htree(hnode_t** t){
    hnode_t* h;
    freq_t low;
    int tLen, i1, i2;

    //Find out how many times we need to go through the frequency table until we're left with one tree
    tLen = 0;
    for(int i = 0; i < FREQ_TABLE_SIZE; i++){
        if(t[i]) tLen++;
    }

    //printf("%d\n",tLen);

    //Here's where the actual tree building begins
    while(tLen > 1){
        i1 = 0;
        i2 = 0;
        low = 18446744073709551615;

        //Find the two trees with the lowest frequencies. I just iterate through the list twice. There might be a more efficient way, but this works, and there's a maximum time complexity, so I don't care
        for(int i = 0; i < FREQ_TABLE_SIZE; i++){
            if(t[i] && t[i]->freq < low){
                i1 = i;
                low = t[i]->freq;
            }
        }

        low = 18446744073709551615;
        for(int i = 0; i < FREQ_TABLE_SIZE; i++){
            if(t[i] && i != i1 && t[i]->freq < low){
                i2 = i;
                low = t[i]->freq;
            }
        }

        //print_hnode(t[i1]);
        //print_hnode(t[i2]);

        //Replace the first lowest node with a new node whose children are the two lowest nodes
        h = create_hnode_fork(t[i1],t[i2]);
        t[i1] = h;
        t[i2] = NULL;

        //print_hnodes(t);

        tLen--;
    }

    //Make sure there's nothing left in the frequency table
    t[i1] = NULL;

    return h;
}

path_t* init_path_table(){
    path_t* p = malloc(sizeof(path_t)*FREQ_TABLE_SIZE);

    for(int i = 0; i < FREQ_TABLE_SIZE; i++){
        p[i].data = 0;
        p[i].len = 0;
    }

    return p;
}

void fill_path_table(hnode_t* h, path_t* p, unsigned int pos, int len){ //Fills up the path lookup table, which is created to make actually writing the compressed file easier
        
    if(h->type == TYPE_DATA){
        p[h->data].data = pos;
        p[h->data].len = len;
    }
    else if(h->type == TYPE_EOF){
        p[FREQ_TABLE_SIZE-1].data = pos;
        p[FREQ_TABLE_SIZE-1].len = len;
    }
    else { //0 means move to the left, 1 means move to the right
        fill_path_table(h->left, p, (pos << 1), len + 1);
        fill_path_table(h->right, p, (pos << 1) + 1, len + 1);
    }
}

void unsigned_int_print_bits(unsigned int i){
    for(int x = 0; x < sizeof(int)*8; x++){
        if((i & 2147483648))printf("1");
        else printf("0");
        i <<= 1;
    }
}

//Write single bits to a file
void write_bit(int f, unsigned int b, int flag){
    static int p = 0;
    static byte_t buffer = 0;

    //Write either a zero or a one to the buffer, depending on what b is
    buffer <<= 1;
    if(b) buffer++;
    //unsigned_int_print_bits((unsigned int)buffer);
    //printf("\n");

    p++;

    if(flag == FLAG_STOPWRITING){ //If you need to stop writing, write the current bits immediately.
        buffer <<= 8 - p;
        //unsigned_int_print_bits((unsigned int)buffer);
        write(f,&buffer,1);
        p = 0;
        buffer = 0;
    } else {
        //If eight bits have been written to the buffer, then write the actual byte to the output file
        if(p >= 8){
            //unsigned_int_print_bits((unsigned int)buffer);
            //printf("\n");
            write(f,&buffer,1);
            p = 0;
            buffer = 0;
        }
    }

    
}

//Write multiple bits at a time
void write_bits(int f, unsigned int b, int len, int flag){
    b <<= 32 - len;
    for(int i = 0; i < len-1; i++){
        write_bit(f, b & 2147483648, 0);
        b <<= 1;
    }
    write_bit(f, b & 2147483648, flag); //We only want the flag to apply with the final bit, since we don't want to prematurely stop writing bits
}

//Read single bits at a time from a file
int read_bit(int f){
    static int p = 0;
    static byte_t b = 0;

    int r = 0;

    //If we need to get more bits from the buffer, get more bits
    if(p == 0 && !read(f,&b,1)) return -1;

    r = ((b & 128)>0) ? 1 : 0;
    b <<= 1;

    p++;
    if(p >= 8) p = 0;

    return r;
}

void write_compressed_file(int in, int out, hnode_t* h, path_t* p){

    byte_t *b;
    
    b = malloc(1);

    //Reset the cursors for both files
    lseek(in,0,SEEK_SET);
    lseek(out,0,SEEK_SET);

    //First, write the huffman tree
    write_tree(out,h);

    //Now, actually write the data
    while(read(in,b,1))write_bits(out,p[*b].data,p[*b].len,0);
    write_bits(out,p[FREQ_TABLE_SIZE-1].data,p[FREQ_TABLE_SIZE-1].len,FLAG_STOPWRITING);

    free(b);

}

//Write the huffman tree to the specified output file
void write_tree(int f, hnode_t* h){

    static byte_t type_data = TYPE_DATA;
    static byte_t type_fork = TYPE_FORK;
    static byte_t type_eof = TYPE_EOF;

    //The nodes will be written as such:
    //If the node is a fork, write TYPE_FORK
    //If the node is data, write TYPE_DATA, and then write the data
    //If the node signifies the EOF, write TYPE_EOF

    if(h->type == TYPE_FORK){
        write(f, &type_fork,1);
        write_tree(f,h->left);
        write_tree(f,h->right);
    }
    else if(h->type == TYPE_DATA){
        write(f, &type_data,1);
        write(f, &(h->data),1);
    }
    else if(h->type == TYPE_EOF){
        write(f, &type_eof, 1);
    }
}


void decompress_file(int in, int out){
    hnode_t* h;
    hnode_t* current;
    byte_t b;
    int keepRunning, r;

    //Reset the cursors for both files
    lseek(in,0,SEEK_SET);
    lseek(out,0,SEEK_SET);

    //Rebuild the huffman tree
    h = create_hnode_from_file(in);

    //print_hnode(h);

    //Now, go through the compressed data and find out
    keepRunning = 1; 
    current = h;
    while(keepRunning){

        //printf("%d\n",current->type);

        if(current->type != TYPE_FORK){ //If it's either data or EOF, write the data to the output file
            if(current->type == TYPE_DATA){
                //printf("%c\n",current->data);
                write(out,&(current->data),1);
            }
            else keepRunning = 0;
            current = h;
        } else { //If it's a fork, then look at the next bit to see whether to move left or right
            r = read_bit(in);

            //This is really bad, but I think that it works well as a failsafe. Once I fix my EOF detection, then this won't be necessary
            if(r<0){
                printf("You shouldn't be seeing this...\n");
                keepRunning = 0;
            }
            else if(r) current = current->right;
            else current = current->left;
            
        }
    }

    free_hnode(h);
}

//Command line stuff

const char *argp_program_version = "0.0.1";
const char *argp_program_bug_address = "<edwards.kyle.j@gmail.com>";

static char doc[] = "A simple huffman compression and decompression program.";

static char args_doc[] = "[file]";

static struct argp_option options[] = {
    {"compress", 'c', 0, 0, "Compress the specified file"},
    {"decompress", 'd', 0, 0, "Decompress the specified file"},
    {"output", 'o', "FILE", 0, "Output to a specific file"},
    {0}
};

struct arguments
{
    char *args[1]; //I think this is correct...
    int toDecompress; //0 for compression, 1 for decompression
    int specificOutfile;
    char *outFile;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state){
    struct arguments *args = state->input;

    switch(key){
        case 'c':
            args->toDecompress = 0;
            break;
        case 'd':
            args->toDecompress = 1;
            break;
        case 'o':
            args->specificOutfile = 1;
            args->outFile = arg;
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 1) argp_usage (state); //Too many arguments

            args->args[state->arg_num] = arg;

            break;
        case ARGP_KEY_END:
            if (state->arg_num < 1) argp_usage (state);//Not enough arguments
            break;
        default:
            return ARGP_ERR_UNKNOWN;
            break;
    }

    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char** argv){

    struct arguments args;

    int input, output;
    double compRatio;

    hnode_t** freqTable;
    hnode_t* huffmanTree;
    path_t* pathTable;

    args.toDecompress = 0;
    args.specificOutfile = 0;

    argp_parse(&argp,argc,argv,0,0,&args);

    //printf("Input file = [%s]\n",args.args[0]);

    input = open(args.args[0],O_RDONLY);

    if(args.specificOutfile){
        output = creat(args.outFile, 0666);
    } else {
        if(args.toDecompress) output = creat("decompressed.txt",0666);
        else output = creat("compressed.huf",0666);
    }

    if(args.toDecompress){
        printf("Decompressing file...\n");
        decompress_file(input,output);
        printf("Decompression complete!\n");
    } else {

        printf("Compressing file...\n");

        printf("Building frequency table...\n");

        pathTable = init_path_table();
        freqTable = init_hnodes(input);

        printf("Building huffman tree...\n");

        huffmanTree = build_htree(freqTable);

        printf("Filling path table...\n");

        fill_path_table(huffmanTree, pathTable, 0, 0);

        printf("Writing to file...\n");

        write_compressed_file(input, output, huffmanTree, pathTable);

        free(freqTable);
        free_hnode(huffmanTree);
        free(pathTable);

        printf("Compression complete!\n");

        //Calculate the compression ratio
        printf("Compression ratio: [%f%%]\n",((double)(lseek(output,0,SEEK_END))/(double)lseek(input,0,SEEK_END))*100);

    }

    close(input);
    close(output);

    return 0;

    /*int output = creat("compressed.huf",0666);

    write_bits(output,0b101001,6,FLAG_STOPWRITING);

    close(output);*/
    
    /*int input, output;
    
    hnode_t** freqTable;
    hnode_t* huffmanTree;
    path_t* pathTable;

    input = open("testInput.txt",O_RDONLY);
    output = creat("testCompressed.txt",0666);

    pathTable = init_path_table();

    freqTable = init_hnodes(input);

    //print_hnodes(t);

    huffmanTree = build_htree(freqTable);

    //print_hnode(huffmanTree);

    fill_path_table(huffmanTree, pathTable, 0, 0);

    //write_bits(output, (unsigned int)'A', 7, FLAG_STOPWRITING);
    write_compressed_file(input,output,huffmanTree,pathTable);

    close(input);
    close(output);

    input = open("testCompressed.txt",O_RDONLY);
    output = creat("testDecompressed.txt",0666);

    //for(int i = 0; i < 8; i++) printf("%d\n",read_bit(input));
    decompress_file(input,output);

    close(input);
    close(output);

    free_hnode(huffmanTree);
    free(pathTable);*/

    return 0;
}