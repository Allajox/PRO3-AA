#ifndef GRAPH_LOGIC_H
#define GRAPH_LOGIC_H

#define SIZE 12

typedef struct {
    int row;
    int col;
} Cell;

// Relative coordinate structure used by the GUI when a '1' is placed
// Relative coordinate structure used by the GUI when a '1' is placed
typedef struct {
    int x;
    int y;
} Coo;

// Graph structure includes adjacency matrix and per-node coordinates for persistence
typedef struct {
    int order;
    int graph[SIZE][SIZE];
    Coo coords[SIZE];
} Graph;

void printGraph(int graph[SIZE][SIZE]);
int saveGraph(const char *filename, const Graph *g);
int loadGraph(const char *filename, Graph *g);
int promising(int graph[SIZE][SIZE], int path[SIZE], int pos);
int hamiltonian(int graph[SIZE][SIZE], int path[SIZE], int pos);

#endif