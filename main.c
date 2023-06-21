#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

typedef struct node node;
typedef struct entry entry;

typedef struct coordinates{ // As all points are 2-D, we only need two coordinates, x and y
    int x; // x coordinate of the point
    int y; // y coordinate of the point
} coordinates;

typedef struct rectangle{
    coordinates min; // Lower left coordinate of the rectangle 
    coordinates max; // Upper right coordinate of the rectangle
} rectangle;

struct entry{
    rectangle *rect; //mbr of the child node
    node* child; //node under this entry
};

struct node{
    int count; // Number of Entries
    bool isLeaf;
    entry **ArrayOfEntries;
    int index; //for testing purposes
    node* parent;
    entry* parententry;
};

typedef struct rTree{
    int maxNumberOfChildren; // maxNumberOfChildren = M
    int minNumberOfChildren; // maxNumberOfChildren = m
    node *root; // Pointer to the root of the tree
} rTree;

coordinates* createCoordinates(int x, int y);
rectangle* createRectangle(coordinates *min, coordinates *max);
node* createNode(bool isLeaf, entry** arr, int index, node* parent, entry* parententry);
entry* createEntry(rectangle* rect, node* child);
rTree* createRTree(int Max, int Min, node* root);
void insert(entry* newval, rTree* ourTree);
node* ChooseLeaf(entry* Entry, node* root);
void adjustTree(node* L, node* LL, node* root, rTree* ourTree);
void quadraticSplit(rTree *parent, node *n, node **split_nodes);
void increaseHeight(node* oldroot1, node* oldroot2, rTree* ourTree);
long long int getArea(rectangle* r1);
rectangle* getMBRofNode(node* n);
rectangle* getMBR(rectangle* r1, rectangle* r2);
long long int getIncreaseInArea(rectangle* r1, rectangle *r2);
void deleteEntry(node *n, int index);
void pickSeeds(node *n, int *indices);
int pickNext(node* n, node* group1, node* group2, bool *firstGroup);
void readData(char *fileName, rTree *tree);
void preOrderTraversal(node *n);
void freeNode(node *n);

coordinates* createCoordinates(int x, int y){
    coordinates* c = malloc(sizeof(coordinates));
    c->x = x;
    c->y = y;
    return c;
}

rectangle* createRectangle(coordinates *min, coordinates *max){
    rectangle* rect = malloc(sizeof(rectangle));
    rect->min.x = min->x;
    rect->min.y = min->y;
    rect->max.x = max->x;
    rect->max.y = max->y;   
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
    if (child) child->parententry = e;
    return e;
}

rTree* createRTree(int Max, int Min, node* root){
    rTree* rt = malloc(sizeof(rTree));
    rt->maxNumberOfChildren = Max; // 4 in our case
    rt->minNumberOfChildren = Min; // 2 in our case
    rt->root = root;
    return rt;
}

int main(void){
    node* root=createNode(true, NULL, 0, NULL, NULL); //creating root node
    root->ArrayOfEntries=calloc(5, sizeof(node*)); 
    rTree* myTree=createRTree(4, 2, root); //creating tree
    readData("data1lakh.txt", myTree); //modify filename here

    preOrderTraversal(myTree->root);
    return 0;
}

void readData(char * fileName, rTree *tree){
    FILE * fp = fopen(fileName, "r");
    int x, y;

    while (fscanf(fp, "%d %d\n", &x, &y) != EOF) {
        insert(createEntry(createRectangle(createCoordinates(x,y), createCoordinates(x,y)), NULL), tree); //calling insert
    }

    fclose(fp);

    return;
}

void insert(entry* newval, rTree* ourTree){
    node* node_to_insert_in = ChooseLeaf(newval, ourTree->root);
    
    if (node_to_insert_in->count<ourTree->maxNumberOfChildren) {        //normal case, with no overflow
        node_to_insert_in->ArrayOfEntries[node_to_insert_in->count]=newval;
        node_to_insert_in->count++;

        if (newval->child) newval->child->parent = node_to_insert_in;

        adjustTree(node_to_insert_in, NULL, ourTree->root, ourTree);
    } else {                                                            //overflow case
        node** split_nodes = calloc(2, sizeof(node*));

        node_to_insert_in->ArrayOfEntries[node_to_insert_in->count] = newval;
        node_to_insert_in->count++;

        if (newval->child) newval->child->parent = node_to_insert_in;

        quadraticSplit(ourTree, node_to_insert_in, split_nodes);        //splitting node after overflow

        if (node_to_insert_in != ourTree->root) node_to_insert_in->parententry->child=split_nodes[0];
        else ourTree->root = split_nodes[0];
        
        adjustTree(split_nodes[0], split_nodes[1], ourTree->root, ourTree);

        free(split_nodes);
        freeNode(node_to_insert_in);
    }

    return;
}

node* ChooseLeaf(entry* e, node* N){
    if (N->isLeaf) return N;
    
    int index = -1;
    long long int minDiff = __LONG_LONG_MAX__;

    for (int i = 0; i < N->count; i++) {
        long long int diff = getIncreaseInArea(N->ArrayOfEntries[i]->rect, e->rect); //finding difference in area
        
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

    while (N != ourTree->root){
        entry* parententry = N->parententry;

        free(parententry->rect);
        parententry->rect = getMBRofNode(N);                                //adjusting MBR of the node
        
        if (split) {                                                        //in case we passed a split node to the adjust function or a split occured along the cascade
            entry* newentry = createEntry(getMBRofNode(NN), NN);
            
            if (N->parent->count < ourTree->maxNumberOfChildren) {          //no overflow in parent node of current iteration (due to split)
                N->parent->ArrayOfEntries[(N->parent->count)] = newentry;
                N->parent->count++;
                NN->parent = N->parent;

                N = N->parent;
                split = false;
            } else {                                                        //overflow in parent node
                node **arrayofnodes = calloc(2, sizeof(node*));
                
                N->parent->ArrayOfEntries[(N->parent->count)] = newentry;
                N->parent->count++;
                NN->parent = N->parent;

                node *old_parent = N->parent;
                
                quadraticSplit(ourTree, N->parent, arrayofnodes);           //splitting overflown parent node

                if (old_parent != ourTree->root) N->parent->parententry->child = arrayofnodes[0]; // replacing N->parent
                else ourTree->root = arrayofnodes[0];

                freeNode(old_parent);

                N = arrayofnodes[0];
                NN = arrayofnodes[1];
                
                split = true;

                free(arrayofnodes);
            }
        } else {
            N = N->parent;
        }
    }

    if (N == ourTree->root && split) increaseHeight(N, NN, ourTree);        //increasing height of the tree in case of root split

    return;
}

void quadraticSplit(rTree *parent, node *n, node **split_nodes) {
    // Gets indices for seeds
    int indices[2];
    pickSeeds(n, indices);

    for (int i = 0; i < 2; i++) {
        // Initializes nodes
        split_nodes[i] = createNode(n->isLeaf, NULL, 0, n->parent, n->parententry);
        split_nodes[i]->ArrayOfEntries = calloc(parent->maxNumberOfChildren+1, sizeof(node*));

        // Seeds nodes
        split_nodes[i]->ArrayOfEntries[0] = n->ArrayOfEntries[indices[i]];
        if (split_nodes[i]->ArrayOfEntries[0]->child) split_nodes[i]->ArrayOfEntries[0]->child->parent = split_nodes[i]; // Re-assigns parent node
        split_nodes[i]->count++;
    }

    // Deletes seed entries from original node
    deleteEntry(n, indices[0]);
    deleteEntry(n, indices[1] - 1); // indices[1] > indices[0], and deleteEntry(n, indices[0]) shifted the required index to the left by 1

    while (n->count > 0) {
        if ((n->count + split_nodes[0]->count <= parent->minNumberOfChildren) || (split_nodes[1]->count >= parent->maxNumberOfChildren)) { // To satisfy minimum and maximum number of children requirement
            for (int i = split_nodes[0]->count; n->count > 0; i++) {
                split_nodes[0]->ArrayOfEntries[i] = n->ArrayOfEntries[0];
                if (split_nodes[0]->ArrayOfEntries[i]->child) split_nodes[0]->ArrayOfEntries[i]->child->parent = split_nodes[0]; // Re-assigns parent node
                split_nodes[0]->count++;
                deleteEntry(n, 0);
            }
        } else if ((n->count + split_nodes[1]->count <= parent->minNumberOfChildren) || (split_nodes[0]->count >= parent->maxNumberOfChildren)) { // To satisfy minimum and maximum number of children requirement
            for (int i = split_nodes[1]->count; n->count > 0; i++) {
                split_nodes[1]->ArrayOfEntries[i] = n->ArrayOfEntries[0];
                if (split_nodes[1]->ArrayOfEntries[i]->child) split_nodes[1]->ArrayOfEntries[i]->child->parent = split_nodes[1]; // Re-assigns parent node
                split_nodes[1]->count++;
                deleteEntry(n, 0);
            }
        } else {
            bool firstGroup;
            int index = pickNext(n, split_nodes[0], split_nodes[1], &firstGroup);
            int node_num = firstGroup ? 0 : 1;

            split_nodes[node_num]->ArrayOfEntries[split_nodes[node_num]->count] = n->ArrayOfEntries[index];
            if (split_nodes[node_num]->ArrayOfEntries[split_nodes[node_num]->count]->child) {
                split_nodes[node_num]->ArrayOfEntries[split_nodes[node_num]->count]->child->parent = split_nodes[node_num]; // Re-assigns parent node
            }
            split_nodes[node_num]->count++;
            deleteEntry(n, index);
        }
    }

    return;
}

void increaseHeight(node* oldroot1, node* oldroot2, rTree* ourTree){
    node* newroot=createNode(false, NULL, 0, NULL, NULL);

    newroot->ArrayOfEntries=calloc(5, sizeof(node*));
    newroot->ArrayOfEntries[0]=createEntry(getMBRofNode(oldroot1), oldroot1);
    newroot->ArrayOfEntries[1]=createEntry(getMBRofNode(oldroot2), oldroot2);

    newroot->count=2;

    oldroot1->parententry=newroot->ArrayOfEntries[0];
    oldroot2->parententry=newroot->ArrayOfEntries[1];

    oldroot1->parent=newroot;
    oldroot2->parent=newroot;

    ourTree->root=newroot;

    return;
}

long long int getArea(rectangle* r1){
    long long int area = (1LL*r1->max.x - 1LL*r1->min.x) * (1LL*r1->max.y - 1LL*r1->min.y);

    if (area < 0) return -area;
    return area;
}

rectangle* getMBRofNode(node* n){
    int minx = INT_MAX, miny = INT_MAX, maxx = INT_MIN, maxy = INT_MIN;

    for(int i=0 ; i<n->count ; i++){
        minx = fmin(minx, n->ArrayOfEntries[i]->rect->min.x);
        minx = fmin(minx, n->ArrayOfEntries[i]->rect->max.x);

        maxx = fmax(maxx, n->ArrayOfEntries[i]->rect->min.x);
        maxx = fmax(maxx, n->ArrayOfEntries[i]->rect->max.x);

        miny = fmin(miny, n->ArrayOfEntries[i]->rect->min.y);
        miny = fmin(miny, n->ArrayOfEntries[i]->rect->max.y);

        maxy = fmax(maxy, n->ArrayOfEntries[i]->rect->min.y);
        maxy = fmax(maxy, n->ArrayOfEntries[i]->rect->max.y);
    }

    rectangle* r = createRectangle(createCoordinates(minx, miny), createCoordinates(maxx, maxy));

    return r;
}

rectangle* getMBR(rectangle* r1, rectangle* r2){
    int minx, miny, maxx, maxy;

    minx = fmin(fmin(r1->min.x, r1->max.x), fmin(r2->min.x, r2->max.x));
    maxx = fmax(fmax(r1->min.x, r1->max.x), fmax(r2->min.x, r2->max.x));

    miny = fmin(fmin(r1->min.y, r1->max.y), fmin(r2->min.y, r2->max.y));
    maxy = fmax(fmax(r1->min.y, r1->max.y), fmax(r2->min.y, r2->max.y));

    rectangle* r = createRectangle(createCoordinates(minx, miny), createCoordinates(maxx, maxy));
    
    return r;
}

long long int getIncreaseInArea(rectangle* r1, rectangle *r2){ // Increase in area of the MBR when rectangle r2 is added to r1
    long long int incr;
    int minx, miny, maxx, maxy;

    maxx = fmax(fmax(r1->max.x, r1->min.x), fmax(r2->max.x, r2->min.x));
    minx = fmin(fmin(r1->max.x, r1->min.x), fmin(r2->max.x, r2->min.x));

    maxy = fmax(fmax(r1->max.y, r1->min.y), fmax(r2->max.y, r2->min.y));
    miny = fmin(fmin(r1->max.y, r1->min.y), fmin(r2->max.y, r2->min.y));

    incr = (1LL*(maxx - minx)*(maxy - miny)) - getArea(r1);
    
    if (incr < 0) return -incr;
    return incr;
}

void deleteEntry(node *n, int index) {
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
    int index_1 = -1, index_2 = -1;
    long long int d = -1;

    for (int i = 0; i < n->count; i++) {
        for (int j = i+1; j < n->count; j++) {
            long long int wasted_area = getArea(getMBR(n->ArrayOfEntries[i]->rect, n->ArrayOfEntries[j]->rect)) - getArea(n->ArrayOfEntries[i]->rect) - getArea(n->ArrayOfEntries[j]->rect);
            
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
    long long int maxIncr = -1;
    int index = -1;
    
    rectangle* r1 = getMBRofNode(group1);
    rectangle* r2 = getMBRofNode(group2);

    for (int i = 0; i < n->count ; i++){
        entry *e = n->ArrayOfEntries[i];
        long long int incr1, incr2;

        incr1 = getIncreaseInArea(r1, e->rect); // Get the increase in area for adding e->rect to r1
        incr2 = getIncreaseInArea(r2, e->rect); // Get the increase in area for adding e->rect to r2

        long long int incr_diff = incr1 - incr2 > 0 ? incr1 - incr2 : incr2 - incr1;

        if(incr_diff >= maxIncr) {
            maxIncr = incr_diff;
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

void preOrderTraversal(node* n) {
    if (n == NULL) return;

    if (n->isLeaf) {
        for (int i = 0; i < n->count; i++) {
            printf("External Node: Objects: (%d, %d)\n", n->ArrayOfEntries[i]->rect->min.x, n->ArrayOfEntries[i]->rect->min.y);
        }
    } else {
        for (int i = 0; i < n->count; i++) {
            printf("\nInternal Node: MBR: (%d, %d) - (%d, %d)\n", n->ArrayOfEntries[i]->rect->min.x, n->ArrayOfEntries[i]->rect->min.y, n->ArrayOfEntries[i]->rect->max.x, n->ArrayOfEntries[i]->rect->max.y);
            preOrderTraversal(n->ArrayOfEntries[i]->child);
        }
    }

    return;
}

void freeNode(node *n) {
    free(n->ArrayOfEntries);
    free(n);

    return;
}