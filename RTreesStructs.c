#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define DIMS 2
#define MAX_ENTRIES 4
#define MIN_ENTRIES 2

enum typeOfNode {
    LEAF = 1,
    INTERNAL = 2,
};

typedef struct rectangle {
    int min[DIMS];
    int max[DIMS];
} rectangle;

typedef struct identifier {
    int id;
} identifier;

struct node {
    enum typeOfNode kind;     // LEAF or BRANCH
    int count;          // number of rectangless
    rectangle rects[MAX_ENTRIES];
    identifier i[MAX_ENTRIES];
    struct node *children[MAX_ENTRIES];
};

typedef struct rtree {
    int count;
    int height;
    struct rectangle rect;
    struct node *root; 
} rtree;
