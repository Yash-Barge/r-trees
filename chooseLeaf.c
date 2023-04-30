#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

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
int  area(rectangle* rect){
    return abs((rect->max.x-rect->min.x)*(rect->max.y-rect->min.y));
}
rectangle* CreateMBR(entry* en,rectangle* rect){


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
//function ChooseLeaf
node* ChooseLeaf(entry* Entry,node* Node){
    if(Node->isLeaf==true){//if we already are at the node we just need to return the node which will contain the entry
        return Node;
    }
    else{//for the non leaf node case we check which  entry  needs the least enlargement. 
        entry* temp;
        int minDiff=INT_MAX;
        int minArr=INT_MAX;
        
        for(int i=0;i<sizeof((Node->ArrayOfEntries));i++){
            int maxX=MAX(Entry->rect->max.x,Node->ArrayOfEntries[i]->rect->max.x);//find the max and min out of the points of the rectangle to be inserted and the entries
            int minX=MIN(Entry->rect->min.x,Node->ArrayOfEntries[i]->rect->min.x);
            int maxY=MAX(Entry->rect->max.y,Node->ArrayOfEntries[i]->rect->max.y);
            int minY=MIN(Entry->rect->min.y,Node->ArrayOfEntries[i]->rect->min.y);
            coordinates* maxc=createCoordinates(maxX,maxY);
            coordinates* minc=createCoordinates(minX,minY);
            rectangle* rect=createRectangle(*minc,*maxc);
            int diff=area(rect)-area(Node->ArrayOfEntries[i]->rect);
            if(diff<minDiff){
                minDiff=diff;
                temp=Node->ArrayOfEntries[i];
                minArr=area(Node->ArrayOfEntries[i]->rect);

                
            }
            else if(diff==minDiff){// if many nodes have the same enlargement we choose the one with the least area
                if(area(Node->ArrayOfEntries[i]->rect)<minArr){
                     temp=Node->ArrayOfEntries[i];
                    minArr=area(Node->ArrayOfEntries[i]->rect);

                   
                    
                }
            }









        }
        return ChooseLeaf(Entry,temp->child);




        
    }
}