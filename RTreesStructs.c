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

struct rectangleCoordinates {
    int min[DIMS];
    int max[DIMS];
};

struct node {
    enum typeOfNode kind;     // LEAF or BRANCH
    int count;          // number of rects
    struct rectangleCoordinates rects[MAX_ENTRIES];
    struct node *children[MAX_ENTRIES];
    
};

struct rtree {
    int count;
    int height;
    struct rectangleCoordinates rect;
    struct node *root; 
};
