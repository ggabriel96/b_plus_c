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

void write(node_t *node, long index);
void read(node_t *dest, long index);

void addFix(node_t **node);
void insert(node_t **tree, long key, long ptr);
void graph(node_t *root);
void graphTree(node_t *tree);
void printTree(node_t *tree);

void throw(char *message) {
    printf("%s\n", message);
    exit(1);
}

node_t * newNode() {
    int i;
    node_t *new = (node_t *) malloc(sizeof (node_t));

    new -> count = 0;
    new -> parent = NIL;
    new -> index = diskIndex++;
    // just initializing everything to "nonexistant"
    for (i = 0; i < B; i++) {
        new -> key[i][INFO] = (long) INFTY;
        new -> key[i][PTR] = NIL;
    }

    return new;
}

void print(node_t *node) {
    int i;

    if (node == NULL) return;

    printf("index: %ld\n", node -> index);
    printf("parent: %ld\n", node -> parent);
    printf("count: %d\n", node -> count);
    for (i = 0; i < B; i++) {
        printf("key[%d]: %ld\n", i, node -> key[i][INFO]);
        printf("ptr[%d]: %ld\n", i, node -> key[i][PTR]);
    }
}

void write(node_t *node, long index) {
    // moving to "index" on "treeFile" file
    // from SEEK_SET (beginning of the file)
    // if successful, fseek returns 0
    if (fseek(treeFile, index * sizeof (node_t), SEEK_SET)) throw("fseek failed @ write()");
    if (fwrite(node, sizeof (node_t), 1, treeFile) == 0) throw("fwrite failed @ write()");
}

void read(node_t *dest, long index) {
    // moving to "index" on "treeFile" file
    // from SEEK_SET (beginning of the file)
    // if successful, fseek returns 0
    if (fseek(treeFile, index * sizeof (node_t), SEEK_SET)) throw("fseek failed @ read()");
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

node_t * find(node_t *node, long key) {
    int i;
    long toRead;

    if (node != NULL) {
        for (i = 0; i < B; i++) {
            if (node -> key[i][INFO] > key) {
                if (i > 0) {
                    printf(">>>> 1\n");
                    toRead = node -> key[i - 1][PTR];
                    node = (node_t *) malloc(sizeof (node_t));
                    read(node, toRead);
                    i = -1;
                }
                else {
                    if (node -> key[i][PTR] == NIL) {
                        printf(">>>> 2\n");
                        return node;
                    }
                    else {
                        printf(">>>> 3\n");
                        read(node, node -> key[i][PTR]);
                        i = -1;
                    }
                }
            }
            else if (node -> key[i][INFO] == key) {
                if (node -> key[i][PTR] == NIL) {
                    return node;
                }
                else {
                    toRead = node -> key[i][PTR];
                    node = (node_t *) malloc(sizeof (node_t));
                    read(node, toRead);
                    i = -1;
                }
            }
        }
    }

    return NULL;
}

void insert(node_t **tree, long key, long ptr) {
    int i;
    node_t *node = NULL;

    if (*tree == NULL) (*tree) = newNode();
    node = find(*tree, key);

    print(node);

    for (i = 0; i < B; i++) {
        if (node -> key[i][INFO] == (long) INFTY) {
            node -> key[i][INFO] = key;
            node -> key[i][PTR] = ptr;
            node -> count++;
            // the matrix to sort, quantity of lines, size of each line, comparing function
            qsort(node -> key, B, sizeof (long) * 2, &cmp);
            break;
        }
    }

    addFix(&node);
    write(node, node -> index);
}

void addFix(node_t **node) {
    int i, j, changedParent = 0;
    long firstKey = NIL;
    node_t *parent = NULL, *left = NULL, *right = NULL, *tmp = NULL;

    if ((*node) -> parent != NIL) {
        parent = (node_t *) malloc(sizeof (node_t));
        read(parent, (*node) -> parent);

        if (parent -> key[0][INFO] > (*node) -> key[0][INFO]) {
            parent -> key[0][INFO] = (*node) -> key[0][INFO];
            changedParent = 1;
        }
    }

    if ((*node) -> count >= B) {
        firstKey = (*node) -> key[0][INFO];

        for (i = 0; i < B / 2; i++) {
            insert(&left, (*node) -> key[i][INFO], (*node) -> key[i][PTR]);

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
            insert(&right, (*node) -> key[i][INFO], (*node) -> key[i][PTR]);

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

        // it's the root of the tree
        if ((*node) -> parent == NIL) {
            free(*node);
            *node = NULL;

            // first keys of left and right
            insert(node, left -> key[0][INFO], left -> index);
            insert(node, right -> key[0][INFO], right -> index);

            left -> parent = (*node) -> index;
            right -> parent = (*node) -> index;

            write((*node), (*node) -> index);
        }
        else {
            delete(&parent, firstKey);
            insert(&parent, left -> key[0][INFO], left -> index);
            insert(&parent, right -> key[0][INFO], right -> index);

            left -> parent = parent -> index;
            right -> parent = parent -> index;

            write(parent, parent -> index);
        }

        write(left, left -> index);
        write(right, right -> index);
    }

    if (parent != NULL && changedParent) {
        write(parent, parent -> index);
        addFix(&parent);
        free(parent);
        parent = NULL;
    }
}

int main(int argc, char const *argv[]) {
    int i;
    char filename[] = "bptree.bin";

    diskIndex = 0;
    root = newNode();
    treeFile = fopen(filename, "w+b");
    if (treeFile != NULL) {
        // insert(root, 7);
        for (i = INS_MAX - 1; i >= 0; i--) {
            insert(&root, i, NIL);
        }
    }
    else {
        printf("Bullshit\n");
    }

    // printTree(root);
    graphTree(root);

    free(root);
    root = NULL;
    return 0;
}

void toString(node_t *node) {
    int i;

    printf("{");
    for (i = 0; i < B && node -> key[i][INFO] != (long) INFTY; i++) {
        printf("%ld", node -> key[i][INFO]);
        if (i + 1 < B && node -> key[i + 1][INFO] != (long) INFTY) printf(", ");
    }
    printf("}");
}

void graph(node_t *node) {
    int i;
    node_t *child = NULL;

    for (i = 0; i < B; i++) {
        if (node -> key[i][PTR] != NIL) {
            child = (node_t *) malloc(sizeof (node_t));
            read(child, node -> key[i][PTR]);

            printf("\t\""); toString(node); printf("\" -> \""); toString(child); printf("\" [label = \"%ld\"];\n", node -> key[i][INFO]);

            graph(child);
            free(child);
            child = NULL;
        }
        // else {
        //     printf("\t\""); toString(node); printf("\" -> \"null\" [label = \"%ld\"];\n", node -> key[i][INFO]);
        // }
    }
}

void graphTree(node_t *tree) {
    printf("digraph BPTree {\n");
    graph(tree);
    printf("}\n");
}

void printTree(node_t *node) {
    int i;
    node_t *child = NULL;

    print(node);

    for (i = 0; i < B; i++) {
        if (node -> key[i][PTR] != NIL) {
            child = (node_t *) malloc(sizeof (node_t));
            read(child, node -> key[i][PTR]);

            printf("%d\n", i);

            printTree(child);
            free(child);
            child = NULL;
        }
    }
}
