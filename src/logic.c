#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "logic.h"

// Auxiliaries
void printPath(int path[], int pathSize) {
    for (int i = 0; i < pathSize; i++) {
        if (i == pathSize - 1)              // if the end was reached, print only the label
            printf("%d", path[i] + 1);
        else
            printf("%d -> ", path[i] + 1);  // +1 to show the correct node label
    }
    printf("\n");
}
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

    // determine file size
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }
    long fsize = ftell(file);
    if (fsize < 0) {
        fclose(file);
        return 0;
    }
    rewind(file);

    // zero the destination structure to provide safe defaults for missing fields
    memset(g, 0, sizeof(Graph));

    // read up to sizeof(Graph) bytes (backwards-compatible if file is smaller)
    size_t to_read = (fsize < (long)sizeof(Graph)) ? (size_t)fsize : sizeof(Graph);
    size_t read_bytes = fread((void*)g, 1, to_read, file);
    fclose(file);

    // clamp order to valid range to avoid out-of-bounds usage elsewhere
    if (g->order < 1) g->order = 1;
    if (g->order > SIZE) g->order = SIZE;

    return (read_bytes == to_read) && (to_read > 0);
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
int hasEulerianPathUndirected(int graph[SIZE][SIZE], int size) {
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
int hasEulerianCycleUndirected(int graph[SIZE][SIZE], int size) {
    for (int i = 0; i < size; i++) {
        int degree = 0;
        for (int j = 0; j < size; j++)
            if (graph[i][j] == 1)
                degree++;

        if (degree % 2 == 1)
            return 0;
    }
    return 1;
}
// directed
int hasEulerianPathDirected(int graph[SIZE][SIZE], int size, int *startVertex) {
    int startNodes = 0, endNodes = 0;
    int startCandidate = -1;

    for (int v = 0; v < size; v++) {
        int indeg = 0, outdeg = 0;

        for (int i = 0; i < size; i++)
            if (graph[i][v] == 1) indeg++;

        for (int j = 0; j < size; j++)
            if (graph[v][j] == 1) outdeg++;

        if (outdeg - indeg == 1) {
            startNodes++;
            startCandidate = v;
        }
        else if (indeg - outdeg == 1) {
            endNodes++;
        }
        else if (indeg != outdeg)
            return 0;
    }

    if (startNodes == 1 && endNodes == 1) {
        *startVertex = startCandidate;
        return 1;
    }

    if (startNodes == 0 && endNodes == 0) {
        // Eulerian cycle → start anywhere with outgoing edge
        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++)
                if (graph[i][j] == 1) {
                    *startVertex = i;
                    return 1;
                }

        return 0;
    }

    return 0;
}
int hasEulerianCycleDirected(int graph[SIZE][SIZE], int size) {
    for (int v = 0; v < size; v++) {
        int indeg = 0, outdeg = 0;

        for (int i = 0; i < size; i++)
            if (graph[i][v] == 1) indeg++;

        for (int j = 0; j < size; j++)
            if (graph[v][j] == 1) outdeg++;

        if (indeg != outdeg)
            return 0;
    }
    return 1;
}

// Hierholzer Algorithm // 

// Stack Methods
void push(Stack *s, int v) {
    s->data[++(s->top)] = v;
}
int pop(Stack *s) {
    return s->data[(s->top)--];
}
int peek(Stack *s) {
    return s->data[s->top];
}
int isEmpty(Stack *s) {
    return s->top == -1;
}
// Hierholzer
int hierholzer(int graph[SIZE][SIZE], int vertexCount, 
    int startVertex, int circuit[], int isDirected, int isConnected) {
    
    if (!isConnected)
        return isConnected;

    // Temp Matriz
    int tempGraph[SIZE][SIZE];
    for (int row = 0; row < vertexCount; row++)
        for (int col = 0; col < vertexCount; col++)
            tempGraph[row][col] = graph[row][col];

    // Stack 
    Stack pathStack;
    pathStack.top = -1;

    // Index for Circuit List
    int circuitIndex = 0;

    // Path 
    push(&pathStack, startVertex);

    while (!isEmpty(&pathStack)) {

        int currentVertex = peek(&pathStack);
        int edgeFound = 0;

        // Look for any edge of this vertex
        for (int nextVertex = 0; nextVertex < vertexCount; nextVertex++) {

            if (tempGraph[currentVertex][nextVertex] == 1) {
                // delete edge or both in case of being undirected
                tempGraph[currentVertex][nextVertex] = 0;
                if (!isDirected)
                    tempGraph[nextVertex][currentVertex] = 0;

                // Avanzar
                push(&pathStack, nextVertex);

                edgeFound = 1;
                break;
            }
        }

        // if there is no edge then add it to the circuit 
        if (!edgeFound) {
            int finishedVertex = pop(&pathStack);
            circuit[circuitIndex++] = finishedVertex;
        }
    }

    // Invert circuit to obtain correct order
    for (int i = 0; i < circuitIndex / 2; i++) {
        int temp = circuit[i];
        circuit[i] = circuit[circuitIndex - 1 - i];
        circuit[circuitIndex - 1 - i] = temp;
    }

    return circuitIndex;
}

// Fleury Algorithm // 

int getDegree(int graph[SIZE][SIZE], int size, int node, int isDirected, int isOut) {
    int deg = 0;

    if (!isDirected) {
        // No dirigido: grado es normal
        for (int j = 0; j < size; j++)
            if (graph[node][j] == 1) deg++;
    } 
    else {
        // Dirigido: elegir outdegree o indegree
        if (isOut) {
            for (int j = 0; j < size; j++)
                if (graph[node][j] == 1) deg++;
        } else {
            for (int i = 0; i < size; i++)
                if (graph[i][node] == 1) deg++;
        }
    }

    return deg;
}
int findRoot(int graph[SIZE][SIZE], int size, int isDirected) {

    if (!isDirected) {
        // === CASO NO DIRIGIDO ===
        int odd = 0, lastOdd = 0;
        for (int i = 0; i < size; i++) {
            if (getDegree(graph, size, i, 0, 1) % 2 != 0) {
                odd++;
                lastOdd = i;
            }
        }
        if (odd == 0) return 0;
        if (odd == 2) return lastOdd;
        return -1;
    }

    // === CASO DIRIGIDO ===
    int start = -1, end = -1;

    for (int i = 0; i < size; i++) {
        int out = getDegree(graph, size, i, 1, 1);
        int in  = getDegree(graph, size, i, 1, 0);

        if (out - in == 1) {
            if (start == -1) start = i;
            else return -1;
        } 
        else if (in - out == 1) {
            if (end == -1) end = i;
            else return -1;
        } 
        else if (in != out)
            return -1;
    }

    if (start != -1) return start;
    return 0;
}
void dfs (int graph[SIZE][SIZE], int size, int start, int visited[], int isDirected) {
    visited[start] = 1;

    for (int i = 0; i < size; i++) {
        if (visited[i]) continue;

        if (isDirected) {
            if (graph[start][i] == 1)
                dfs(graph, size, i, visited, isDirected);
        } else {
            if (graph[start][i] == 1 || graph[i][start] == 1)
                dfs(graph, size, i, visited, isDirected);
        }
    }
}
// helper: returns 1 if node has any incident edge (outgoing or incoming for directed)
int nodeHasEdges(int graph[SIZE][SIZE], int size, int node, int isDirected) {
    if (!isDirected) {
        for (int j = 0; j < size; j++)
            if (graph[node][j] == 1)
                return 1;
        return 0;
    }

    for (int j = 0; j < size; j++) {
        if (graph[node][j] == 1) return 1;
        if (graph[j][node] == 1) return 1;
    }
    return 0;
}

// helper: count reachable vertices from `start` that still have at least one incident edge
int countReachableNodesWithEdges(int graph[SIZE][SIZE], int size, int start, int isDirected) {
    int visited[SIZE] = {0};
    int cnt = 0;
    dfs(graph, size, start, visited, isDirected);
    for (int i = 0; i < size; i++)
        if (visited[i] && nodeHasEdges(graph, size, i, isDirected))
            cnt++;
    return cnt;
}
int isBridge(int graph[SIZE][SIZE], int size, int u, int v, int isDirected) {
    // If the edge is the only outgoing edge from u, it's forced (must take it)
    int outdeg = getDegree(graph, size, u, isDirected, 1);
    if (outdeg == 1)
        return 0; // not a bridge for Fleury's choice
    // For undirected graphs test reachability from u (original behaviour).
    // For directed graphs we must test reachability from v (the vertex we'll move to),
    // because after taking the edge u->v we will be at v and need to reach remaining edges.
    int startNode = isDirected ? v : u;

    // Count reachable vertices that still have incident edges before removal
    int before = countReachableNodesWithEdges(graph, size, startNode, isDirected);

    // remove edge temporarily
    graph[u][v] = 0;
    if (!isDirected) graph[v][u] = 0;

    int after = countReachableNodesWithEdges(graph, size, startNode, isDirected);

    // restore edge
    graph[u][v] = 1;
    if (!isDirected) graph[v][u] = 1;

    return (after < before);
}
int getNextNode(int graph[SIZE][SIZE], int size, int node, int isDirected) {

    for (int j = 0; j < size; j++) {
        if (graph[node][j] == 1) {
            if (!isBridge (graph, size, node, j, isDirected))
                return j;
        }
    }

    for (int j = 0; j < size; j++)
        if (graph[node][j] == 1)
            return j;

    return -1;
}
int isCompleted(int graph[SIZE][SIZE], int size, int isDirected) {
    for (int i = 0; i < size; i++)
        if (getDegree(graph, size, i, isDirected, 1) > 0)
            return 0;
    return 1;
}
int fleury(int graph[SIZE][SIZE], int path[], int size, int *pathSize, int root, int isConnected, int isDirected) {

    if (!isConnected) return 0;

    int current = root;
    path[0] = current;
    *pathSize = 1;

    for (;;) {
        int next = getNextNode (graph, size, current, isDirected);

        if (next == -1) break;

        path[*pathSize] = next;
        (*pathSize)++;

        // remove edge
        graph[current][next] = 0;
        if (!isDirected) graph[next][current] = 0;

        current = next;

        if (isCompleted (graph, size, isDirected))
            return 1;
    }

    return isCompleted (graph, size, isDirected);
}

/*int getDegree(int graph[SIZE][SIZE], int size, int node) {
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
void dfs(int graph[SIZE][SIZE], int size, int start, int visited[]) {
// backtracking algorithm (Depth first search)
    visited[start] = 1; // marks starting node as visited

    for (int i = 0; i < size; i++)
        // if there's an edge between start and i, and i has not been visited (i == 0),
        // go recursively to that new node
        if (graph[start][i] == 1 && !visited[i])
            dfs(graph, size, i, visited);
}
int isBridge(int graph[SIZE][SIZE], int size, int current, int next) {
// uses dfs to determine if the current edge is a bridge
    int visited[SIZE] = {0};    // stores visited nodes
    int componentsBefore = 0, componentsAfter = 0;

    // counts how many components were there before removing an edge
    dfs(graph, size, current, visited);
    for (int i = 0; i < size; i++)
        componentsBefore += visited[i];

    // removes edge temporarily
    graph[current][next] = 0; 
    graph[next][current] = 0;

    // clears the array
    for (int i = 0; i < size; i++) 
        visited[i] = 0;

    // counts the components once again after removing the edge
    dfs(graph, size, current, visited);
    for (int i = 0; i < size; i++)
        componentsAfter += visited[i];

    // restores the edges
    graph[current][next] = 1;
    graph[next][current] = 1;

    return componentsAfter < componentsBefore;  // if fewer nodes reachable, it's a bridge
}
int getNextNode(int graph[SIZE][SIZE], int size, int node) {
    for (int i = 0; i < size; i++) {
        if (graph[node][i] == 1) {
            // avoids bridges
            if (!isBridge(graph, size, node, i))
                return i;
        }
    }
    // if all the connected nodes are bridges, choose the first encountered
    for (int i = 0; i < size; i++) {
        if (graph[node][i] == 1)
            return i;
    }
    return -1;
}
int isCompleted(int graph[SIZE][SIZE], int size) {
// determines if the graph has no more edges
    for (int i = 0; i < size; i++) {
        if (getDegree(graph, size, i) > 0)
            return 0;
    }
    return 1;
}
// Fleury
int fleury(int graph[SIZE][SIZE], int path[], int size, int *pathSize, int root, int isConnected) {   
    if (!isConnected)
        return isConnected;

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

*/




/*int main() {
    int size = 3;
    int graph[SIZE][SIZE] = {
        {0,1,0},
        {0,0,1},
        {0,0,0},
    };
    int circuit[SIZE * SIZE];
    int len = hierholzer(graph, size, 0, circuit, 1);
    printf("Circuito Euleriano:\n");
    for (int i = 0; i < len; i++)
        printf("%d ", circuit[i]);
    printf("\n");
    return 0;
}
*/