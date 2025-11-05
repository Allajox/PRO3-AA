#include <stdio.h>
#include "logic.h"

void printGraph(int graph[SIZE][SIZE], int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++)
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

// function that checks if the graph has a hamiltonian 
// cycle or path, identified by a mode (0 for cycle, 1 for path)
int hamiltonian(int graph[SIZE][SIZE], int path[SIZE], int size, int pos, int mode) {
    // if the path is full
    if (pos == size) {
        if (mode == 0)  // cycle mode
            // returns true if the last node connects with the first. A solution was found
            return graph[path[pos - 1]][path[0]];
        else            // path mode
            return 1;   // doesn't check the last node
    }
    
    // tries all nodes
    for (int i = 0; i < size; i++) {
        path[pos] = i; // tries the current node (i)

        if (promising(graph, path, pos)) { // checks if the node is promising
            if (hamiltonian(graph, path, size, pos + 1, mode)) // checks the next node
                return 1;
                
            path[pos] = -1; // if the path doesn't end in a solution, backtrack
        }
    }    
    return 0;
}

int eulerian(int graph[SIZE][SIZE], int size) {
    int odd = 0;

    for (int i = 0; i < size; i++) {
        int count = 0;
        for(int j = 0; j < size; j++) {
            // stores the amount of vertices connected with 1
            if (graph[i][j] == 1)
                count++;
        }
        // if i has and odd number of connected vertices, increase by 1
        if (count % 2 == 1)
            odd++;
    }

    // if 
    if (odd > 2)
        return 0;
    else 
        return 1;
}

/**/
int main() {
    // HAS PATH AND CYCLE
    int graph[SIZE][SIZE] = {
        {0, 1, 0, 1, 0}, 
        {1, 0, 1, 1, 1}, 
        {0, 1, 0, 0, 1}, 
        {1, 1, 0, 0, 1}, 
        {0, 1, 1, 1, 0}
    };

    // NO PATH AND NO CYCLE
    int graph1[SIZE][SIZE] = {
        {0, 1, 1, 0, 0},
        {1, 0, 1, 0, 0},
        {1, 1, 0, 0, 0},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 1, 0}
    };

    // HAS PATH BUT NO CYCLE
    int graph2[SIZE][SIZE] = {
        {0, 1, 0, 0, 0},
        {1, 0, 1, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 1, 0, 1},
        {0, 0, 0, 1, 0}
    };

    // DOESN'T HAVE EULER PATH
    int graph3[SIZE][SIZE] = {
        {0, 1, 0, 0, 0},
        {1, 0, 1, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 1, 0}
    };


    int path[SIZE];
    path[0] = 0;

    printGraph(graph2, 5);
    if (eulerian(graph2, 5))
        printf("Has euler path\n");
    else
        printf("No euler path\n");

    if (hamiltonian(graph2, path, 5, 1, 0)) {
        printf("Hamiltonian cycle found: ");
        // prints the cycle
        for (int i = 0; i < 5; i++)
            printf("%d ", path[i]);
        printf("%d ", path[0]);
        printf("\n");
    } else {
        printf("No cycle found\n");
    }

    if (hamiltonian(graph2, path, 5, 0, 1)) {
        printf("Hamiltonian path found: ");
        // prints the cycle
        for (int i = 0; i < 5; i++)
            printf("%d ", path[i]);
        printf("\n");
    } else {
        printf("No path found\n");
    }

    return 0;
}
