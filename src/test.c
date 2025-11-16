#include <stdio.h>
#include "logic.h"
#include <time.h>
#include <stdlib.h>


/*int main() {
    int graph[SIZE][SIZE] = {
        {0, 1, 0, 1, 0, 0},
        {1, 0, 1, 0, 0, 0},
        {0, 1, 0, 1, 0, 0},
        {1, 0, 1, 0, 1, 1},
        {0, 0, 0, 1, 0, 1},
        {0, 0, 0, 1, 1, 0}
    };

    int size = 6;
    int pathSize;
    
    // the path can have maximum (size * (size - 1)) / 2 edges, so for this project,
    // the maximum is 66 (gotten with 12 nodes)
    int path[66];

    int root = findRoot(graph, size);
    
    if (root != -1) {
        if (hasEulerianCycleUndirected(graph, size))
            printf("Available Eulerian Circuit is\n");

        else if (hasEulerianPathUndirected(graph, size, &root))
            printf("Available Eulerian Path is\n");

        if (fleury(graph, path, size, &pathSize, root))
            printPath(path, pathSize);
        else
            printf("Couldn't complete the eulerian path or circuit.\n");
    } else
        printf("Eulerian path or circuit not available\n");
    
    return 0;
}*/