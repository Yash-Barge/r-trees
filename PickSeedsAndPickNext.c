#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

typedef struct node node;
typedef struct entry entry;

typedef struct coordinates { // As all points are 2-D, we only need two coordinates, x and y
    int x; // x coordinate of the point
    int y; // y coordinate of the point
} coordinates;

typedef struct rectangle {
    coordinates min; // Lower left coordinate of the rectangle 
    coordinates max; // Upper right coordinate of the rectangle
} rectangle;

struct entry {
    rectangle *rect;
    node* child;
};

struct node {
    int count; // Number of Entries
    bool isLeaf;
    entry **ArrayOfEntries;
    int index;
};

typedef struct rTree {
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

rectangle* createRectangle(coordinates* min, coordinates* max){
    rectangle* rect = malloc(sizeof(rectangle));
    rect->min.x = min->x;
    rect->min.y = min->y;
    rect->max.x = max->x;
    rect->max.y = max->y;
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

int getArea(rectangle* r1){
    int area = (r1->max.x - r1->min.x) * (r1->max.y - r1->min.y);
    return abs(area);
}

rectangle* getMBRofNode(node* n){
    if(n->count == 0){
        printf("\nWARNING: Number of Entries in the Node is 0\n");
        printf("\nFunction Name: getMBRofNode\n");
        return NULL;
    }

    int minx = INT_MAX, miny = INT_MAX, maxx = INT_MIN, maxy = INT_MIN;
    for(int i=0 ; i<n->count ; i++){
        minx = fmin(minx, n->ArrayOfEntries[i]->rect->min.x);
        minx = fmin(minx, n->ArrayOfEntries[i]->rect->max.x);
        maxx = fmax(minx, n->ArrayOfEntries[i]->rect->min.x);
        maxx = fmax(minx, n->ArrayOfEntries[i]->rect->max.x);
        miny = fmin(minx, n->ArrayOfEntries[i]->rect->min.y);
        miny = fmin(minx, n->ArrayOfEntries[i]->rect->max.y);
        maxy = fmax(minx, n->ArrayOfEntries[i]->rect->min.y);
        maxy = fmax(minx, n->ArrayOfEntries[i]->rect->max.y);
    }
    coordinates* min = createCoordinates(minx, miny);
    coordinates* max = createCoordinates(maxx, maxy);
    rectangle* r = createRectangle(min, max);
    return r;
}

rectangle* getMBR(rectangle* r1, rectangle* r2){
    int minx, miny, maxx, maxy;

    minx = fmin(fmin(r1->min.x, r1->max.x), fmin(r2->min.x, r2->max.x));
    maxx = fmax(fmax(r1->min.x, r1->max.x), fmax(r2->min.x, r2->max.x));
    miny = fmin(fmin(r1->min.y, r1->max.y), fmin(r2->min.y, r2->max.y));
    maxy = fmax(fmax(r1->min.y, r1->max.y), fmax(r2->min.y, r2->max.y));

    coordinates* c1 = createCoordinates(minx, miny);
    coordinates* c2 = createCoordinates(maxx, maxy);
    rectangle* r = createRectangle(c1, c2);
    
    return r;
}

int getIncreaseInArea(rectangle* r1, rectangle *r2){ // Increase in area of the MBR when rectangle r2 is added to r1
    int incr;
    int minx, miny, maxx, maxy;

    maxx = fmax(fmax(r1->max.x, r1->min.x), fmax(r2->max.x, r2->min.x));
    minx = fmin(fmin(r1->max.x, r1->min.x), fmin(r2->max.x, r2->min.x));
    maxy = fmax(fmax(r1->max.y, r1->min.y), fmax(r2->max.y, r2->min.y));
    miny = fmin(fmin(r1->max.y, r1->min.y), fmin(r2->max.y, r2->min.y));

    incr = ((maxx - minx)*(maxy - miny)) - getArea(r1);
    return incr;
}

void deleteEntry(node *n, int index) {
    if (index >= n->count) {
        printf("\nWARNING: Attempting to delete entry at out-of-bounds index!\n");
        printf("Node entry count: %d\nIndex at attempted deletion: %d\n", n->count, index);
        return;
    }

    for (int i = index + 1; i < n->count; i++) {
        n->ArrayOfEntries[i - 1] = n->ArrayOfEntries[i];
    }

    n->ArrayOfEntries[n->count - 1] = NULL;
    n->count--;

    return;
}

// Adjust Tree (Already Written by VVD)

// Choose Leaf
node* ChooseLeaf(entry* Entry,node* Node){
    if(Node->isLeaf==true){//if we already are at the node we just need to return the node which will contain the entry
        return Node;
    }
    else{//for the non leaf node case we check which  entry  needs the least enlargement. 
        entry* temp;
        int minDiff=__INT_MAX__;
        int minArr=__INT_MAX__;
        
        for(int i=0;i<sizeof((Node->ArrayOfEntries));i++){
            int maxX=fmax(Entry->rect->max.x,Node->ArrayOfEntries[i]->rect->max.x);//find the max and min out of the points of the rectangle to be inserted and the entries
            int minX=fmin(Entry->rect->min.x,Node->ArrayOfEntries[i]->rect->min.x);
            int maxY=fmax(Entry->rect->max.y,Node->ArrayOfEntries[i]->rect->max.y);
            int minY=fmin(Entry->rect->min.y,Node->ArrayOfEntries[i]->rect->min.y);
            coordinates* maxc=createCoordinates(maxX,maxY);
            coordinates* minc=createCoordinates(minX,minY);
            rectangle* rect=createRectangle(minc, maxc);
            int diff=getArea(rect)-getArea(Node->ArrayOfEntries[i]->rect);
            if(diff<minDiff){
                minDiff=diff;
                temp=Node->ArrayOfEntries[i];
                minArr=getArea(Node->ArrayOfEntries[i]->rect);
            }
            else if(diff==minDiff){// if many nodes have the same enlargement we choose the one with the least area
                if(getArea(Node->ArrayOfEntries[i]->rect)<minArr){
                    temp=Node->ArrayOfEntries[i];
                    minArr=getArea(Node->ArrayOfEntries[i]->rect);       
                }
            }
        }
        return ChooseLeaf(Entry,temp->child);       
    }
}

// Pick Seeds function
// Takes in a pointer to a node and an array of integers of at least size 2
// Returns nothing, but the array of integers will have the indices of both entries of the least efficient pair
void pickSeeds(node *n, int *indices) {
    int index_1 = -1, index_2 = -1, d = -1;
    const entry *entries = *(n->ArrayOfEntries);

    for (int i = 0; i < n->count; i++) {
        for (int j = i+1; j < n->count; j++) {
            int wasted_area = getArea(getMBR(entries[i].rect, entries[j].rect)) - getArea(entries[i].rect) - getArea(entries[j].rect);
            
            if (wasted_area > d) {
                d = wasted_area;
                index_1 = i;
                index_2 = j;
            }
        }
    }

    indices[0] = index_1;
    indices[1] = index_2;
    
    return;
}

// Pick Next Function

// Takes inputs of a pointer to a node (which is being split), and 2 groups (which it is being split into)
// For each entry of in the ArrayOfEntries of the input node, check for which entry gives the minimum area 
// difference when adding the rectangles to the MBRs 
// It also takes a bool pointer input, which is set to true or false based on which group the entry needs to be added to

int pickNext(node* n, node* group1, node* group2, bool *firstGroup){
    int minIncr = 0;
    int index = -1;
    
    rectangle* r1 = getMBRofNode(group1);
    rectangle* r2 = getMBRofNode(group2);

    for (int i = 0; i < n->count ; i++){
        entry *e = n->ArrayOfEntries[i];
        int incr1, incr2;

        incr1 = getIncreaseInArea(r1, e->rect); // Get the increase in area for adding e->rect to r1
        incr2 = getIncreaseInArea(r2, e->rect); // Get the increase in area for adding e->rect to r2

        if(abs(incr1 - incr2) >= minIncr){
            minIncr = abs(incr1 - incr2);
            index = i;  
            if (incr1 == incr2) {
                if (getArea(r1) != getArea(r2)) {
                    *firstGroup = (getArea(r1) < getArea(r2));
                } 
                else {
                    *firstGroup = (group1->count < group2->count);
                }
            }
            else {
                *firstGroup = (incr1 < incr2);
            }
        }
    }
    return index;
}

// Quadratic Split
// Takes in a pointer to the parent rTree, pointer to node which is to be split, and an array of pointers to nodes of at least size 2
// Does not return anything, but adds 2 pointers to the split_nodes parameter array
void quadraticSplit(rTree *parent, node *n, node **split_nodes) {
    // Guard clause
    if (n->count < (parent->minNumberOfChildren * 2)) {
        printf("\nWARNING: Cannot perform quadratic split!\n");
        printf("Node does not have enough entries to split between 2 nodes and satisfy minimum number of children requirements!");
        printf("Node entry count: %d\nMinimum number of children of rTree: %d\n", n->count, parent->minNumberOfChildren);

        return;
    } else if (n->count > (parent->maxNumberOfChildren * 2)) {
        printf("\nFATAL ERROR: Quadratic split\n");
        printf("Node has more entries than can split between 2 nodes and satisfy maximum number of children requirements!");
        printf("Node entry count: %d\nMaximum number of children of rTree: %d\n", n->count, parent->maxNumberOfChildren);

        exit(1);
    }

    // Gets indices for seeds
    int indices[2];
    pickSeeds(n, indices);

    for (int i = 0; i < 2; i++) {
        // Initializes nodes
        split_nodes[i] = createNode(n->isLeaf, NULL, 0);
        split_nodes[i]->ArrayOfEntries = calloc(parent->maxNumberOfChildren, sizeof(node*));

        // Seeds nodes
        split_nodes[i]->ArrayOfEntries[0] = n->ArrayOfEntries[indices[i]];
        split_nodes[i]->count++;
    }

    // Deletes seed entries from original node
    deleteEntry(n, indices[0]);
    deleteEntry(n, indices[1] - 1); // indices[1] > indices[0], and deleteEntry(n, indices[0]) shifted the required index to the left by 1

    while (n->count > 0) {
        if ((n->count + split_nodes[0]->count <= parent->minNumberOfChildren) || (split_nodes[1]->count >= parent->maxNumberOfChildren)) { // To satisfy minimum and maximum number of children requirement
            for (int i = split_nodes[0]->count; n->count > 0; i++) {
                split_nodes[0]->ArrayOfEntries[i] = n->ArrayOfEntries[0];
                split_nodes[0]->count++;
                deleteEntry(n, 0);
            }
        } else if ((n->count + split_nodes[1]->count <= parent->minNumberOfChildren) || (split_nodes[0]->count >= parent->maxNumberOfChildren)) { // To satisfy minimum and maximum number of children requirement
            for (int i = split_nodes[1]->count; n->count > 0; i++) {
                split_nodes[1]->ArrayOfEntries[i] = n->ArrayOfEntries[0];
                split_nodes[1]->count++;
                deleteEntry(n, 0);
            }
        } else {
            bool firstGroup;
            int index = pickNext(n, split_nodes[0], split_nodes[1], &firstGroup);
            int node_num = firstGroup ? 0 : 1;

            split_nodes[node_num]->ArrayOfEntries[split_nodes[node_num]->count] = n->ArrayOfEntries[index];
            split_nodes[node_num]->count++;
            deleteEntry(n, index);
        }
    }

    // Terminates program if something goes wrong
    if (split_nodes[0]->count < parent->minNumberOfChildren || split_nodes[1]->count < parent->minNumberOfChildren) {
        printf("\nFATAL ERROR: Quadratic split\n");
        printf("Split nodes do not have enough entries to satisfy minimum children requirements!\n");
        printf("split_nodes[0]->count: %d\nsplit_nodes[1]->count: %d\nMinimum number of children for rTree: %d\n", split_nodes[0]->count, split_nodes[1]->count, parent->minNumberOfChildren);
        printf("Exiting program...\n");
        exit(1);
    } else if (split_nodes[0]->count > parent->maxNumberOfChildren || split_nodes[1]->count > parent->maxNumberOfChildren) {
        printf("\nFATAL ERROR: Quadratic split\n");
        printf("Split nodes have too many entries to satisfy maximum children requirements!\n");
        printf("split_nodes[0]->count: %d\nsplit_nodes[1]->count: %d\nMaximum number of children for rTree: %d\n", split_nodes[0]->count, split_nodes[1]->count, parent->maxNumberOfChildren);
        printf("Exiting program...\n");
        exit(1);
    }
    return;
}

// driver function

int main(void) {
    printf("%d", INT_MIN);
}
