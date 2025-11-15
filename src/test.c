#include <stdio.h>
#include "logic.h"
#include <time.h>
#include <stdlib.h>

void printPath(int path[], int pathSize) {
    for (int i = 0; i < pathSize; i++) {
        if (i == pathSize - 1)              // if the end was reached, print only the label
            printf("%d", path[i] + 1);
        else
            printf("%d -> ", path[i] + 1);  // +1 to show the correct node label
    }
    printf("\n");
}

int getDegree(int graph[SIZE][SIZE], int size, int node) {
    int deg = 0;
    for (int j = 0; j < size; j++) {
        if (graph[node][j] == 1) deg++; 
    }
    return deg;
}

int findRoot(int graph[SIZE][SIZE], int size) {
    int lastOdd = 0; // start from the first node
    int oddCount = 0;
    
    for (int i = 0; i < size; i++) {
        if (getDegree(graph, size, i) % 2 != 0) {
            oddCount++;
            lastOdd = i;
        }
    }
    // if the conditions are not met, stop
    if (oddCount != 0 && oddCount != 2)
        return -1;
        
    // if there's two odd nodes, return the last one visited
    else if (oddCount == 2)
        return lastOdd;

    // if there are no odd nodes, choose one at random
    else {
        srand(time(NULL));
        return rand() % size;
    } 
}

int getNextNode(int graph[SIZE][SIZE], int size, int node) {
    for (int i = 0; i < size; i++) {   
        if (graph[node][i] == 1) {
            // avoids bridges (degree = 1)
            if (getDegree(graph, size, i) != 1)
                return i;
        }
    }
    // if all the connected nodes are bridges, choose the first encountered
    for (int i = 0; i < size; i++) {   
        if (graph[node][i] == 1) {
            return i;
        }
    }
    return -1;
}

// determines if the graph has no more edges
int isCompleted(int graph[SIZE][SIZE], int size) {
    for (int i = 0; i < size; i++) {
        if (getDegree(graph, size, i) > 0)
            return 0;
    }
    return 1;
}

// main algorithm
int fleury(int graph[SIZE][SIZE], int path[], int size, int *pathSize, int root) {   
    path[0] = root;
    int current = root;
    *pathSize = 1;

    // formula for the maximum amount of edges
    int maxEdges = (size * (size - 1)) / 2;

    // loop that inserts the nodes into the path
    for (int i = 0; i < maxEdges; i++) {
        int next = getNextNode(graph, size, current);
        if (next == -1) {
            if (!isCompleted(graph, size)) 
                return 0;
            else
                break;
        }

        path[i + 1] = next;
        (*pathSize)++;
        // removes the edges from the graph
        graph[current][next] = 0;
        graph[next][current] = 0;

        // updates current
        current = next;
        
        if (isCompleted(graph, size))
            return 1;
    }
    return (isCompleted(graph, size));
}

int main() {
    int graph[SIZE][SIZE] = {
        {0, 1, 0, 0, 1},
        {1, 0, 1, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 1, 0, 1},
        {1, 0, 0, 1, 0}
    };

    /*int graph2[SIZE][SIZE] = {
        {0, 1, 1, 1, 1},
        {1, 0, 1, 1, 1},
        {1, 1, 0, 1, 1},
        {1, 1, 1, 0, 1},
        {1, 1, 1, 1, 0}
    };

    int graph3[SIZE][SIZE] = {
        {0, 1, 1, 0, 1, 1},
        {1, 0, 1, 1, 0, 1},
        {1, 1, 0, 1, 1, 0},
        {0, 1, 1, 0, 1, 1},
        {1, 0, 1, 1, 0, 1},
        {1, 1, 0, 1, 1, 0}
    };*/

    int size = 5;
    int pathSize;
    
    // the path can have maximum (size * (size - 1)) / 2 edges, so for this project,
    // the maximum is 66 (gotten with 12 nodes)
    int path[66];

    int root = findRoot(graph, size);
    
    if (root != -1) {
        if (eulerianPath(graph, size))
            printf("Available Eulerian Path is\n");

        else if (eulerianCycle(graph, size))
            printf("Available Eulerian Circuit is\n");

        if (fleury(graph, path, size, &pathSize, root))
            printPath(path, pathSize);
        else
            printf("Couldn't complete the eulerian path or circuit.\n");
    } else
        printf("Eulerian path or circuit not available\n");
    
    return 0;
}