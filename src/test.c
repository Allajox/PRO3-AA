#include <stdio.h>
#include "logic.h"
#include <time.h>
#include <stdlib.h>

void displayPath(int path[], int pathSize) {
    for (int i = 0; i < pathSize; i++)
        if (i == pathSize - 1) // if the end was reached, don't print the arrow
            printf("%d", path[i] + 1);
        else
            printf("%d -> ", path[i] + 1); // +1 to show the correct node label
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
    int current = 0; // start from the first node
    int oddCount = 0;
    
    for (int i = 0; i < size; i++) {
        if (getDegree(graph, size, i) % 2 != 0) {
            oddCount++;
            current = i;
        }
    }
    // if the conditions are not met, stop
    if (oddCount != 0 && oddCount != 2)
        return -1;
        
    else if (oddCount == 2)
        return current;

    // if there are no odd nodes, choose one at random
    else {
        srand(time(NULL));
        return rand() % size;
    } 
}

int isLastNode(int graph[SIZE][SIZE], int size) {
    int degSum = 0;
    for (int i = 0; i < size; i++)
        degSum += getDegree(graph, size, i);
    return (degSum == 2);
}

int hasOneEdge(int graph[SIZE][SIZE], int size, int node) {  
    return (getDegree(graph, size, node) == 1);
}

int getNextNode(int graph[SIZE][SIZE], int size, int node) {
    for (int i = 0; i < size; i++) {   
        if (graph[node][i] == 1) {
            if (!hasOneEdge(graph, size, i))
                return i;
            else {
                if (isLastNode(graph, size))
                    return i;
            }
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
int fleury(int graph[SIZE][SIZE], int path[], int size, int root) {   
    path[0] = root;
    int current = root;
    int next;
    int pathSize = 1;

    // formula for the maximum amount of edges
    int maxEdges = (size * (size - 1)) / 2;

    // loop that inserts the nodes into the path
    for (int i = 0; i < maxEdges; i++) {
        next = getNextNode(graph, size, current);
        if (next == -1 || isCompleted(graph, size)) 
            break; 

        path[i] = next;
        pathSize++;
        // removes the edges from the graph
        graph[current][next] = 0;
        graph[next][current] = 0;

        // updates current
        current = next;
    }
    return pathSize;
}

int countOddDegreeNodes(int graph[SIZE][SIZE], int size) {
    int oddCount = 0;
    for (int i = 0; i < size; i++) {
        if (getDegree(graph, size, i) % 2 != 0)
            oddCount++;
    }
    return oddCount;
}

int main() {
    int graph[SIZE][SIZE] = {
        {0, 1, 0, 0, 1},
        {1, 0, 1, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 1, 0, 1},
        {1, 0, 0, 1, 0}
    };

    /*int graph[SIZE][SIZE] = {
        {0, 1, 1, 1, 1},
        {1, 0, 1, 1, 1},
        {1, 1, 0, 1, 1},
        {1, 1, 1, 0, 1},
        {1, 1, 1, 1, 0}
    };*/

    /*int graph[SIZE][SIZE] = {
        {0, 1, 1, 0, 1, 1},
        {1, 0, 1, 1, 0, 1},
        {1, 1, 0, 1, 1, 0},
        {0, 1, 1, 0, 1, 1},
        {1, 0, 1, 1, 0, 1},
        {1, 1, 0, 1, 1, 0}
    };*/

    int size = 5;
    
    // the path can have maximum (size * (size - 1)) / 2 nodes, so for this project,
    // the maximum is 66 (gotten with 12 nodes)
    int path[66];

    int root = findRoot(graph, size);
    int oddCount = countOddDegreeNodes(graph, size);
    
    if (root != -1) {
        if (oddCount == 2)  // semi eulerian (path)
            printf("Available Eulerian Path is\n");
        else                // eulerian (cycle)
            printf("Available Eulerian Circuit is\n");

        path[0] = root;
        int pathSize = fleury(graph, path, size, root);

        displayPath(path, pathSize);
    } else
        printf("Eulerian path or circuit not available\n");
    
    return 0;
}