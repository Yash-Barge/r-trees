#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

typedef struct node node;
typedef struct entry entry;

typedef struct coordinates{ // As all points are 2d-D, we only need two coordinates, x and y
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


void preOrderTraversal(node* n) {
    
    if (n == NULL) {
        return;
    }


    if (n->isLeaf) {
        for (int i = 0; i < n->count; i++) {
            printf("External Node: Objects: (%d, %d)\n", n->ArrayOfEntries[i]->rect->min.x, n->ArrayOfEntries[i]->rect->min.y);
        }
    } 
    
    else {
        
        for (int i = 0; i < n->count; i++) {
            printf("Internal Node: MBR: (%d, %d) - (%d, %d)\n", n->ArrayOfEntries[i]->rect->min.x, n->ArrayOfEntries[i]->rect->min.y, n->ArrayOfEntries[i]->rect->max.x, n->ArrayOfEntries[i]->rect->max.y);
            preOrderTraversal(n->ArrayOfEntries[i]->child);
        }
    }
}

int main()
{
    return 0;
}

