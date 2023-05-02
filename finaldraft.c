#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

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
    node* parent;
    entry* parententry;
} node;

typedef struct rTree{
    int maxNumberOfChildren; // maxNumberOfChildren = M
    int minNumberOfChildren; // maxNumberOfChildren = m
    //entry *start; 
    node *root; // Pointer to the root of the tree
} rTree;

coordinates* createCoordinates(int x, int y);
rectangle* createRectangle(coordinates min, coordinates max);
node* createNode(bool isLeaf, entry** arr, int index, node* parent, entry* parententry);
entry* createEntry(rectangle* rect, node* child);
rTree* createRTree(int Max, int Min, node* root);
void insert(entry* newval, rTree* ourTree);
node* ChooseLeaf(entry* Entry, node* root);
void adjustTree(node* L, node* LL, node* root, rTree* ourTree);
void quadraticSplit(rTree *parent, node *n, node **split_nodes);
void increaseHeight(node* oldroot1, node* oldroot2, rTree* ourTree);
int getArea(rectangle* r1);
rectangle* getMBRofNode(node* n);
rectangle* getMBR(rectangle* r1, rectangle* r2);
int getIncreaseInArea(rectangle* r1, rectangle *r2);
void deleteEntry(node *n, int index);
void pickSeeds(node *n, int *indices);
int pickNext(node* n, node* group1, node* group2, bool *firstGroup);

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

node* createNode(bool isLeaf, entry** arr, int index, node* parent, entry* parententry){
    node* n = malloc(sizeof(node));
    n->ArrayOfEntries = arr;
    n->isLeaf = isLeaf;
    n->index = index;
    n->count = 0;
    n->parent=parent;
    n->parententry=parententry;
    return n;
}

entry* createEntry(rectangle* rect, node* child){
    entry* e = malloc(sizeof(entry));
    e->child = child;
    e->rect = rect;
    return e;
}

rTree* createRTree(int Max, int Min, node* root){
    rTree* rt = malloc(sizeof(rTree));
    rt->maxNumberOfChildren = Max; // 4 in our case
    rt->minNumberOfChildren = Min; // 2 in our case
    //rt->start = start;
    rt->root = root;
    return rt;
}


int main(void){
    node* root=createNode(true, NULL, 0, NULL, NULL);
    root->ArrayOfEntries=calloc(5, sizeof(node*));
    rTree* myTree=createRTree(4, 2, root);
    readData("data.txt", myTree);
}

void readData(char * fileName, rTree *tree){
    FILE * fp = fopen(fileName, "r");

    int x, y;

    while (fscanf(fp, "%d %d\n", &x, &y) != EOF) {
        insert(createEntry(createRectangle(createCoordinates(x,y), createCoordinates(x,y)), NULL), tree);
    }

    fclose(fp);

    return;
}

void insert(entry* newval, rTree* ourTree){
    node* node_to_insert_in = ChooseLeaf(newval, ourTree->root);
    
    if (node_to_insert_in->count<ourTree->maxNumberOfChildren) {
        node_to_insert_in->ArrayOfEntries[node_to_insert_in->count]=newval;
        node_to_insert_in->count++;
        adjustTree(node_to_insert_in, NULL, ourTree->root, ourTree);
    } else {
        node** split_nodes = calloc(2, sizeof(node*));
        node_to_insert_in->ArrayOfEntries[node_to_insert_in->count]=newval;
        node_to_insert_in->count++;
        quadraticSplit(ourTree, node_to_insert_in, split_nodes);
        node_to_insert_in->parententry->child=split_nodes[0];
        adjustTree(split_nodes[0], split_nodes[1], ourTree->root,ourTree);

        free(split_nodes);
        // TODO: free node_to_insert
    }

    return;
}

node* ChooseLeaf(entry* e, node* N){
    if (N->isLeaf) return N;
    
    int index = -1;
    int minDiff = INT_MAX;

    for (int i = 0; i < N->count; i++) {
        int diff = getIncreaseInArea(N->ArrayOfEntries[i]->rect, e->rect);
        
        if ((diff < minDiff) || ((diff == minDiff) && (getArea(N->ArrayOfEntries[i]->rect) < getArea(N->ArrayOfEntries[index]->rect)))) {
            minDiff = diff;
            index = i;
        }
    }

    return ChooseLeaf(e, N->ArrayOfEntries[index]->child);
}

void adjustTree(node* L, node* LL, node* root, rTree* ourTree) {
    bool split = (LL != NULL);

    node *N = L;
    node *NN = LL;

    while (N != root){
        entry* parententry = N->parententry;

        // TODO: free parententry->rect
        parententry->rect = getMBRofNode(N);
        
        if (split) {
            entry* newentry = createEntry(getMBRofNode(NN), NN);
            
            if (N->parent->count < ourTree->maxNumberOfChildren) {
                N->parent->ArrayOfEntries[(N->parent->count)] = newentry;
                N->parent->count++;

                N = N->parent;
                split = false;
            } else {
                node **arrayofnodes = calloc(2, sizeof(node*));
                
                N->parent->ArrayOfEntries[(N->parent->count)] = newentry;
                N->parent->count++;
                
                quadraticSplit(ourTree, N->parent, arrayofnodes);

                node *temp = N->parent;
                N->parent->parententry->child = arrayofnodes[0]; // replacing N->parent
                // TODO: free temp

                N = arrayofnodes[0];
                NN = arrayofnodes[1];
                
                split = true;

                free(arrayofnodes);
            }
        } else {
            N = N->parent;
        }
    }

    if (N == root && split) increaseHeight(N, NN, ourTree);

    return;
}

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
        split_nodes[i] = createNode(n->isLeaf, NULL, 0, n->parent, n->parententry);
        split_nodes[i]->ArrayOfEntries = calloc(parent->maxNumberOfChildren+1, sizeof(node*));

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

void increaseHeight(node* oldroot1, node* oldroot2, rTree* ourTree){
    node* newroot=createNode(false, NULL, 0, NULL, NULL);
    newroot->ArrayOfEntries=calloc(5, sizeof(node*));
    newroot->ArrayOfEntries[0]->child=oldroot1;
    newroot->ArrayOfEntries[1]->child=oldroot2;
    newroot->count=2;
    newroot->ArrayOfEntries[0]->rect=getMBRofNode(oldroot1);
    newroot->ArrayOfEntries[1]->rect=getMBRofNode(oldroot2);
    oldroot1->parententry=newroot->ArrayOfEntries[0];
    oldroot2->parententry=newroot->ArrayOfEntries[1];
    oldroot1->parent=newroot;
    oldroot2->parent=newroot;
    ourTree->root=newroot;
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
    if (index >= n->count || index < 0) {
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
    int maxIncr = -1;
    int index = -1;
    
    rectangle* r1 = getMBRofNode(group1);
    rectangle* r2 = getMBRofNode(group2);

    for (int i = 0; i < n->count ; i++){
        entry *e = n->ArrayOfEntries[i];
        int incr1, incr2;

        incr1 = getIncreaseInArea(r1, e->rect); // Get the increase in area for adding e->rect to r1
        incr2 = getIncreaseInArea(r2, e->rect); // Get the increase in area for adding e->rect to r2

        if(abs(incr1 - incr2) >= maxIncr){
            maxIncr = abs(incr1 - incr2);
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
