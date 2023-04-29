#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct coordinates{ // As all points are 2-D, we only need two coordinates, x and y
    int x; // x coordinate of the point
    int y; // y coordinate of the point
} coordinates;

typedef struct rectangle{
    coordinates min; // Lower left coordinate of the rectangle 
    coordinates max; // Upper right coordinate of the rectangle
} rectangle;

typedef struct entry{
    rectangle *rect;
    node* child;
} entry;

typedef struct node{
    int count; // Number of Entries
    bool isLeaf;
    entry **ArrayOfEntries;
    int index;
} node;

typedef struct rTree{
    int maxNumberOfChildren; // maxNumberOfChildren = M
    int minNumberOfChildren; // maxNumberOfChildren = m
    entry *start; 
    node *root; // Pointer to the root of the tree
} rTree;


coordinates* createCoordinates(int x, int y){
    coordinates* c = malloc(sizeof(coordinates));
    c->x = x;
    c->y = y;
    return c;
}

rectangle* createRectangle(coordinates min, coordinates max){
    rectangle* rect = malloc(sizeof(rectangle));
    rect->min.x = min.x;
    rect->min.y = min.y;
    rect->max.x = max.x;
    rect->max.y = max.y;
    return rect;
}

node* createNode(bool isLeaf, entry** arr, int index){
    node* n = malloc(sizeof(node));
    n->ArrayOfEntries = arr;
    n->isLeaf = isLeaf;
    n->index = index;
    n->count = 0;
    return n;
}

entry* createEntry(rectangle* rect, node* child){
    entry* e = malloc(sizeof(entry));
    e->child = child;
    e->rect = rect;
    return e;
}

rTree* createRTree(int Max, int Min, entry* start, node* root){
    rTree* rt = malloc(sizeof(rTree));
    rt->maxNumberOfChildren = Max; // 4 in our case
    rt->minNumberOfChildren = Min; // 2 in our case
    rt->start = start;
    rt->root = root;
    return rt;
}
