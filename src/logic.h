#ifndef GRAPH_LOGIC_H
#define GRAPH_LOGIC_H

#define SIZE 5

typedef struct {
    int graph[SIZE][SIZE];
} Graph;

typedef struct {
    int row;
    int col;
} Cell;

void printGraph(int graph[SIZE][SIZE]);
int saveGraph(const char *filename, const Graph *g);
int loadGraph(const char *filename, Graph *g);
int promising(int graph[SIZE][SIZE], int path[SIZE], int pos);
int hamiltonian(int graph[SIZE][SIZE], int path[SIZE], int pos);

#endif