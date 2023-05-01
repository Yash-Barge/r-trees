#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

typedef struct coordinates { // As all points are 2-D, we only need two coordinates, x and y
    int x; // x coordinate of the point
    int y; // y coordinate of the point
} coordinates;

typedef struct rectangle {
    coordinates min; // Lower left coordinate of the rectangle 
    coordinates max; // Upper right coordinate of the rectangle
} rectangle;

typedef struct node node;
typedef struct entry entry;

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

// Takes inputs of a pointer to a node, and 2 pointers to Rectangles, which are MBRs
// For each entry of in the ArrayOfEntries of the input node, check for which entry gives the minimum area 
// difference when adding the rectangles to the MBRs 

int pickNext(node* n, rectangle* r1, rectangle* r2){
    entry* e;
    int incr1, incr2, minIncr = __INT_MAX__;
    int area1, area2, minArea;
    int index = -1;

    for(int i=0 ; i< n->count ; i++){
        e = n->ArrayOfEntries[i];

        rectangle* r3 = getMBR(r1, e->rect);
        area1 = getArea(r3); // get Area of MBR of r1 and e->rect
        
        rectangle* r4 = getMBR(r2, e->rect);
        area2 = getArea(r4); // get Area of MBR of r2 and e->rect

        incr1 = getIncreaseInArea(r1, e->rect); // Get the increase in area for adding e->rect to r1
        incr2 = getIncreaseInArea(r2, e->rect); // Get the increase in area for adding e->rect to r2

        if(abs(incr1 - incr2) < minIncr){
            minIncr = abs(incr1 - incr2);
            minArea = area1 + area2;
            index = i;
        } 
        else if ((abs(incr1 - incr2) == minIncr) && (area1 + area2 < minArea)) {
            minArea = area1 + area2;
            index = i;
        }
    }
    return index;
}

// Quadratic Split

void quadraticSplit(node *n, node **e) {
    
}

// driver function

int main(void) {
    printf("Work it\n");
}
