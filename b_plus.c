#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define B 4
#define NIL (-1L)

#define INS_MAX 10

FILE *treeFile;

typedef struct BPNode {
    int count;
    int key[B];
    long index[B + 1];
} node_t;

void throw(char *message) {
    printf("%s\n", message);
    exit(1);
}

void write(node_t node) {

}

void read(node_t *dest, long index) {
    // moving to "index" on "treeFile" file
    // from SEEK_SET (beginning of the file)
    // if successful, fseek returns 0
    if (fseek(treeFile, index, SEEK_SET)) throw("fseek failed, line 31");
    // reads 1 node_t from "treeFile" and stores it into "dest"
    // fread returns the number of elements read
    if ((fread(dest, sizeof (node_t), 1, treeFile)) == 0) throw("fread failed, line 33");
}

void insert(node_t tree, int key) {

}

int main(int argc, char const *argv[]) {
    int i;
    node_t root;
    char filename[] = {'t', 'r', 'e', 'e', '.', 't', 'x', 't'}; // ?

    treeFile = fopen(filename, "w+b");
    if (treeFile != NULL) {
        for (i = 0; i < INS_MAX; i++) {
            insert(root, i);
        }
    }
    else {
        printf("Bullshit\n");
    }

    return 0;
}
