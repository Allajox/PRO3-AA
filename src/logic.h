#ifndef GRAPH_LOGIC_H
#define GRAPH_LOGIC_H

#define SIZE 12
#include <stdio.h>

// Stack
typedef struct {
    int data[SIZE * SIZE];
    int top;
} Stack;

// Cell
typedef struct {
    int row;
    int col;
} Cell;

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
    
    int startNode;
    int circuit[SIZE];

    int isConnected;
    int isDirected;
    int isEulerian;
    int isSemiEulerian;
    int hasHamiltonPath;
    int hasHamiltonCycle;
} Graph;


void printGraph(int graph[SIZE][SIZE], int size);
int saveGraph(const char *filename, const Graph *g);
int loadGraph(const char *filename, Graph *g);

int promising(int graph[SIZE][SIZE], int path[SIZE], int pos);
int hamiltonian(int graph[SIZE][SIZE], int path[SIZE], int size, int pos, int mode);
int isConnected(int graph[SIZE][SIZE], int size);

int hasEulerianPathUndirected(int graph[SIZE][SIZE], int size, int *startVertex);
int hasEulerianCycleUndirected(int graph[SIZE][SIZE], int size);

int hasEulerianPathDirected(int graph[SIZE][SIZE], int size, int *startVertex);
int hasEulerianCycleDirected(int graph[SIZE][SIZE], int size);

int hierholzer(int graph[SIZE][SIZE], int size, int start, int circuit[], int isDirected);

#endif