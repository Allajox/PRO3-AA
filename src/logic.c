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
    size_t written = fwrite(g, sizeof(Graph), 1, file);
    fclose(file);

    return (written == 1); // 1 if the file was opened, 0 if error
}
int loadGraph(const char *filename, Graph *g) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
        return 0;
    size_t read = fread(g, sizeof(Graph), 1, file);
    fclose(file);

    return (read == 1); // 1 if the file was opened, 0 if error
}

// Just 1 "Island"
int hasIsolatedVertex(int graph[SIZE][SIZE], int size) {
    for (int i = 0; i < size; i++) {
        int connected = 0;
        for (int j = 0; j < size; j++) {
            if (graph[i][j] || graph[j][i]) {
                connected = 1;
                break;
            }
        }
        if (!connected)
            return 1; // there's an isolated vertex
    }
    return 0;
}
int isFullyReachable(int graph[SIZE][SIZE], int size) {
    int visited[SIZE] = {0};
    int stack[SIZE];
    int top = 0;

    visited[0] = 1;
    stack[top++] = 0;

    while (top > 0) {
        int v = stack[--top];
        for (int u = 0; u < size; u++) {
            if (!visited[u] && (graph[v][u] || graph[u][v])) {
                visited[u] = 1;
                stack[top++] = u;
            }
        }
    }

    for (int i = 0; i < size; i++)
        if (!visited[i])
            return 0;

    return 1;
}
int isConnected(int graph[SIZE][SIZE], int size) {
    if (hasIsolatedVertex(graph, size))
        return 0;
    return isFullyReachable(graph, size);
}

// Hamilton (Directed and Not Directed // Cycle or Path)
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

// Euler
// undirected
int eulerianPath(int graph[SIZE][SIZE], int size) {
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

    // if there's exactly 2 nodes of odd degree, is semi-eulerian
    if (odd == 2)
        return 1;
    else 
        return 0;
}
int eulerianCycle(int graph[SIZE][SIZE], int size) {
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

    if (odd > 0)
        return 0;
    else 
        return 1;
}
int eulerianPathDirected(int graph[SIZE][SIZE], int size) { // O(n²) cost?
    int indegree = 0, outdegree = 0;

    // for each node
    for (int v = 0; v < size; v++) {
        int count_in = 0, count_out = 0;
        
        // count 1s on rows and then columns
        for (int i = 0; i < size; i++)
            if (graph[i][v] == 1) count_in++;
        for (int j = 0; j < size; j++)
            if (graph[v][j] == 1) count_out++;

        // output - input is 1 => indegree = 1
        if (count_out - count_in == 1)
            indegree++;
        // input - output is 1 => outdegree = 1
        else if (count_in - count_out == 1)
            outdegree++;
        // all other vertices have the same out degree and in degree
        else if (count_in != count_out)
            return 0;

        // if there's more than 1 node with indegree - outdegree = 1
        // or outdegree - indegree = 1, the graph is no semi Eulerian
        if (indegree > 1 || outdegree > 1)
            return 0;
    }

    // if the conditions are met, return true, it's semi Eulerian
    return ( (indegree == 0 && outdegree == 0) || (indegree == 1 && outdegree == 1) );
}
int eulerianCycleDirected(int graph[SIZE][SIZE], int size) {
    for (int v = 0; v < size; v++) {
        
        int count_in = 0, count_out = 0;
        
        for(int i = 0; i < size; i++) {
            if (graph[i][v] == 1)
                count_in++;
        }
        
        for(int j = 0; j < size; j++) {
            if (graph[v][j] == 1)
                count_out++;
        }
        
        if (count_in != count_out)
            return 0;
    }
    return 1;
}

/*
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

    int graph4[SIZE][SIZE] = {
        {0, 1, 0, 0}, 
        {0, 0, 1, 0}, 
        {1, 0, 0, 0}, 
        {0, 0, 0, 0}, 
    };


    int path[SIZE];
    path[0] = 0;

    printGraph(graph, 5);
    if (eulerianPathDirected(graph, 5))
        printf("Has euler path\n");
    else
        printf("No euler path\n");

    if (eulerianCycleDirected(graph, 5))
        printf("It's Eulerian Graph\n");
    else 
        printf("It's not Eulerian Graph\n");

    if (hamiltonian(graph, path, 5, 1, 0)) {
        printf("Hamiltonian cycle found: ");
        // prints the cycle
        for (int i = 0; i < 4; i++)
            printf("%d ", path[i]);
        printf("%d ", path[0]);
        printf("\n");
    } else {
        printf("No cycle found\n");
    }

    if (hamiltonian(graph, path, 5, 0, 1)) {
        printf("Hamiltonian path found: ");
        // prints the cycle
        for (int i = 0; i < 4; i++)
            printf("%d ", path[i]);
        printf("\n");
    } else {
        printf("No path found\n");
    }

    if (isConnected(graph, 5))
        printf("It is connected\n");
    else
        printf("It is NOT connected\n");

    return 0;
}
*/