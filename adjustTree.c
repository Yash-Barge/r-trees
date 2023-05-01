#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int maxNumberOfChildren; // maxNumberOfChildren = M
int minNumberOfChildren; // maxNumberOfChildren = m

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

rTree* createRTree(int Max, int Min, entry* start, node* root){
    rTree* rt = malloc(sizeof(rTree));
    rt->maxNumberOfChildren = Max; // 4 in our case
    rt->minNumberOfChildren = Min; // 2 in our case
    rt->start = start;
    rt->root = root;
    return rt;
}

void adjustTree(node* L, node* LL, node* root){
    coordinates* least = malloc(sizeof(coordinates)); //check if malloc is necessary
    coordinates* most = malloc(sizeof(coordinates));
    node* N= malloc(sizeof(node));
    node* NN= malloc(sizeof(node));
    //node* P= malloc(sizeof(node));
    //node* PP= malloc(sizeof(node));
    node** arrayofnodes;
    bool split;
    if(LL==null){
        split=false;
    }
    else{
        split=true;
    }
    N=L;
    NN=LL;
    entry* parententry=N->parententry;
    entry* newentry=malloc(sizeof(entry));
    rectangle* newentryrect=malloc(sizeof(rectangle));
    while(N!=root){
        least=findMin(N);
        most=findMax(N);
        parententry->rect->min=least;
        parententry->rect->max=most;
        
        if(split){          
            newentryrect->max=findMax(NN);
            newentryrect->min=findMin(NN);
            newentry=createEntry(newentryrect, NN);
            
            if(N->parent->count<maxNumberOfChildren){
                N->parent->ArrayOfEntries[(N->parent->count)]=newentry;

                N=N->parent
                split=false;                
            }
            else{
                arrayofnodes=splitNode(N->parent, newentry);
                N=arrayofnodes[0];
                NN=arrayofnodes[1];
                split=true;            
            }
        }            
        else
        {
            N=N->parent;
        }
    }
}

coordinates* findMin(node* L){ //finds most bottom-left coordinate in the entries array of a node, ie the lower limit
    int n=sizeof(L->ArrayOfEntries)/sizeof(L->ArrayOfEntries[0]); //finding number of elements in node
    coordinates* least = malloc(sizeof(coordinates));
    least= L->ArrayOfEntries[0]->rect->min; //initializing least
    coordinates* check = malloc(sizeof(coordinates));
    for(int i=0; i<n; i++){
        check=L->ArrayOfEntries[i]->rect->min;
        if(check->x<=least->x && check->y<=least->y){
            least=check;
        }
    }
    return least;

}

coordinates* findMax(node* L){ //finds most top-right coordinate in the entries array of a node, ie the upper limit
    int n=sizeof(L->ArrayOfEntries)/sizeof(L->ArrayOfEntries[0]); //finding number of elements in node
    coordinates* most = malloc(sizeof(coordinates));
    most= L->ArrayOfEntries[0]->rect->max; //initializing most
    coordinates* check = malloc(sizeof(coordinates));
    for(int i=0; i<n; i++){
        check=L->ArrayOfEntries[i]->rect->max;
        if(check->x>=most->x && check->y>=most->y){
            most=check;
        }
    }
    return most;

}