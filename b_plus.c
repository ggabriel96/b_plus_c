#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define B 4
#define NIL (-1L)
// INFTY tells us that this is a "null node"
#define INFTY 2e32
#define INS_MAX 4

#define INFO 0
#define PTR 1

typedef struct BPNode {
    int count;
    long index, parent, key[B][2];
} node_t;

FILE *treeFile;
node_t *root;
long diskIndex;

void addFix(node_t **node);
void insert(node_t **tree, long key);

void throw(char *message) {
    printf("%s\n", message);
    exit(1);
}

void print(node_t *node) {
    int i;
    printf("count: %d\n", node -> count);
    printf("parent: %ld\n", node -> parent);
    for (i = 0; i < B; i++) {
        printf("key[%d]: %ld\n", i, node -> key[i][INFO]);
        printf("ptr[%d]: %ld\n", i, node -> key[i][PTR]);
    }
}

void write(node_t *node, long index) {
    // moving to "index" on "treeFile" file
    // from SEEK_SET (beginning of the file)
    // if successful, fseek returns 0
    if (fseek(treeFile, index, SEEK_SET)) throw("fseek failed @ write()");
    if (fwrite(node, sizeof (node_t), 1, treeFile) == 0) throw("fwrite failed @ write()");
}

void read(node_t *dest, long index) {
    // moving to "index" on "treeFile" file
    // from SEEK_SET (beginning of the file)
    // if successful, fseek returns 0
    if (fseek(treeFile, index, SEEK_SET)) throw("fseek failed @ read()");
    // reads 1 node_t from "treeFile" and stores it into "dest"
    // fread returns the number of elements read
    if ((fread(dest, sizeof (node_t), 1, treeFile)) == 0) throw("fread failed @ read()");
}

int signalOf(long x) {
    if (x == 0) return 0;
    else return (int) (x / labs(x));
}

int cmp(const void *a, const void *b) {
	return signalOf((*(long *)a - *(long *)b));
}

void delete(node_t **tree, long key) {
    int i;

    for (i = 0; i < B; i++) {
        if ((*tree) -> key[i][INFO] == key) {
            (*tree) -> key[i][INFO] = (long) INFTY;
            (*tree) -> key[i][PTR] = NIL;
            (*tree) -> count--;
            // the matrix to sort, quantity of lines, size of each line, comparing function
            qsort((*tree) -> key, B, sizeof (long) * 2, &cmp);
            break;
        }
    }

    // delFix();
}

void insert(node_t **tree, long key) {
    int i;

    if (*tree == NULL) {
        (*tree) = (node_t *) malloc(sizeof (node_t));

        // when I don't yet have a tree, the first element (root)
        // will have 1 key/info pointing to nowhere (because it is
        // a leaf, which contains the information itself)
        (*tree) -> count = 1;
        (*tree) -> index = diskIndex++;
        (*tree) -> parent = NIL;
        (*tree) -> key[0][INFO] = key;
        (*tree) -> key[0][PTR] = NIL;

        // just initializing everything to "nonexistant"
        for (i = 1; i < B; i++) {
            (*tree) -> key[i][INFO] = (long) INFTY;
            (*tree) -> key[i][PTR] = NIL;
        }

        // writing it to disk at position 0 (L is for long)
        // because it is the root
        write(*tree, 0L);
    }
    else {
        for (i = 0; i < B; i++) {
            if ((*tree) -> key[i][INFO] == (long) INFTY) {
                (*tree) -> key[i][INFO] = key;
                (*tree) -> key[i][PTR] = NIL;
                (*tree) -> count++;
                // the matrix to sort, quantity of lines, size of each line, comparing function
                qsort((*tree) -> key, B, sizeof (long) * 2, &cmp);
                break;
            }
        }

        addFix(tree);
    }
}

void addFix(node_t **node) {
    int i, j;
    long firstKey = NIL;
    node_t *parent = NULL, *left = NULL, *right = NULL, *tmp = NULL;

    if ((*node) -> parent != NIL) {
        parent = (node_t *) malloc(sizeof (node_t));
        read(parent, (*node) -> parent);

        if (parent -> key[0][INFO] > (*node) -> key[0][INFO]) {
            parent -> key[0][INFO] = (*node) -> key[0][INFO];
        }
    }

    if ((*node) -> count >= B) {
        firstKey = (*node) -> key[0][INFO];

        for (i = 0; i < B / 2; i++) {
            insert(&left, (*node) -> key[i][INFO]);
            left -> key[i][PTR] = (*node) -> key[i][PTR];

            // updating children's parent
            if (left -> key[i][PTR] != NIL) {
                tmp = (node_t *) malloc(sizeof (node_t));
                read(tmp, left -> key[i][PTR]);

                tmp -> parent = left -> index;
                write(tmp, left -> key[i][PTR]);

                free(tmp);
                tmp = NULL;
            }
        }

        for (j = 0; i < B; i++, j++) {
            insert(&right, (*node) -> key[i][INFO]);
            right -> key[j][PTR] = (*node) -> key[i][PTR];

            // updating children's parent
            if (right -> key[i][PTR] != NIL) {
                tmp = (node_t *) malloc(sizeof (node_t));
                read(tmp, right -> key[i][PTR]);

                tmp -> parent = right -> index;
                write(tmp, right -> key[i][PTR]);

                free(tmp);
                tmp = NULL;
            }
        }

        if ((*node) -> parent == NIL) {
            // tmp = NULL;
            // first keys of left and right
            insert(&tmp, left -> key[0][INFO]);
            insert(&tmp, right -> key[0][INFO]);

            left -> parent = tmp -> index;
            right -> parent = tmp -> index;

            root = tmp;
            tmp = NULL;
        }
        else {
            delete(&parent, firstKey);
            insert(&parent, left -> key[0][INFO]);
            insert(&parent, right -> key[0][INFO]);

            left -> parent = parent -> index;
            right -> parent = parent -> index;
        }

        write(left, left -> index);
        write(right, right -> index);

        free(*node);
        *node = NULL;
        node = NULL;
    }

    printf("> PARENT:\n");
    if (parent != NULL) print(parent);
    printf("> LEFT:\n");
    if (left != NULL) print(left);
    printf("> RIGHT:\n");
    if (right != NULL) print(right);

    if (parent != NULL) {
        write(parent, parent -> index);
        addFix(&parent);
        free(parent);
        parent = NULL;
    }
}

int main(int argc, char const *argv[]) {
    int i;
    char filename[] = "tree.txt";

    root = NULL;
    diskIndex = 0;
    treeFile = fopen(filename, "w+b");
    if (treeFile != NULL) {
        // insert(root, 7);
        for (i = INS_MAX - 1; i >= 0; i--) {
            insert(&root, i);
            printf("------\n");
            print(root);
            printf("------\n");
        }
    }
    else {
        printf("Bullshit\n");
    }

    return 0;
}
