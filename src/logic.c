#include<stdio.h>

#define SIZE 12

typedef struct {
    int order;
    int graph[SIZE][SIZE];
} Graph;

typedef struct {
    int row;
    int col;
} Cell;

void printGraph(int graph[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++)
            printf("%d ", graph[i][j]);
        printf("\n");
    }
}

int saveGraph(const char *filename, const Graph *g) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
        return 0;
    size_t escritos = fwrite(g, sizeof(Graph), 1, file);
    fclose(file);

    return (escritos == 1); // 1 si se abrió el archivo, 0 si fue error
}

int loadGraph(const char *filename, Graph *g) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
        return 0;
    size_t leidos = fread(g, sizeof(Graph), 1, file);
    fclose(file);

    return (leidos == 1); // 1 si se abrió el archivo, 0 si fue error
}

int promising(int graph[SIZE][SIZE], int path[SIZE], int pos) {
    if (pos > 0 && !graph[path[pos - 1]][path[pos]])
        return 0;

    // checks if the node in the current position is repeated, if true, returns false
    for (int i = 0; i < pos; i++) {
        if (path[i] == path[pos])
            return 0;
    }
    return 1;
}

int hamiltonian(int graph[SIZE][SIZE], int path[SIZE], int pos) {
    // if the path is full
    if (pos == SIZE) {
        // returns true if the last node connects with the first. A solution was found
        return graph[path[pos - 1]][path[0]];
    }
    
    // tries all nodes
    for (int i = 1; i < SIZE; i++) {
        path[pos] = i; // tries the current node (i)

        if (promising(graph, path, pos)) { // checks if the node is promising
            if (hamiltonian(graph, path, pos + 1)) // checks the next node
                return 1;
                
            path[pos] = -1; // if the path doesn't end in a solution, backtrack
        }
    }    
    return 0;
}

/*
int main() {
    // CYCLE = 0 1 2 4 3 0
    int graph[SIZE][SIZE] = {
        {0, 1, 0, 1, 0}, 
        {1, 0, 1, 1, 1}, 
        {0, 1, 0, 0, 1}, 
        {1, 1, 0, 0, 1}, 
        {0, 1, 1, 1, 0}
    };

    // CYCLE = 0 1 2 3 4 0
    int graph1[SIZE][SIZE] = {
    {0, 1, 0, 0, 1},
    {1, 0, 1, 0, 0},
    {0, 1, 0, 1, 0},
    {0, 0, 1, 0, 1},
    {1, 0, 0, 1, 0}
};

    // NO CYCLE
    int graph2[SIZE][SIZE] = {
        {0, 1, 0, 0, 0},
        {1, 0, 1, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 1, 0, 1},
        {0, 0, 0, 1, 0}
    };


    int path[SIZE];
    path[0] = 0;

    if (hamiltonian(graph, path, 1)) {
        printGraph(graph);
        printf("Hamiltonian cycle found: ");
        // prints the cycle
        for (int i = 0; i < SIZE; i++)
            printf("%d ", path[i]);

        printf("%d\n", path[0]); // completes the cycle with the first node
    } else {
        printf("No solution found\n");
    }

    return 0;
}
*/