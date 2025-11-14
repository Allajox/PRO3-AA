#include<stdio.h>
#include"logic.h"

int fp = 0, count = 0;

void displayPath(int finalPath[SIZE]) {
    for(int i = 0; i < fp; i++)
        printf("%d -> ", finalPath[i]);
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
    int current = 0; // Empezar desde el primer nodo
    count = 0; // Reiniciar count
    
    for (int i = 0; i < size; i++) {
        if(getDegree(graph, size, i) % 2 != 0) {
            count++;
            current = i;
        }
    }
    
    if (count != 0 && count != 2)
        return 0;
    else 
        return current;
}

int isHasOneEdge(int graph[SIZE][SIZE], int size, int node) {  
    return (getDegree(graph, size, node) == 1);
}

int isLastNode(int graph[SIZE][SIZE], int size) {
    int degSum = 0;
    for(int i = 0; i < size; i++)
        degSum += getDegree(graph, size, i);
    return (degSum == 2);
}

int getNextNode(int graph[SIZE][SIZE], int size, int node) {
    for (int i = 0; i < size; i++) {   
        if (graph[node][i] == 1) {
            if (!isHasOneEdge(graph, size, i))
                return i;
            else {
                if (isLastNode(graph, size))
                    return i;
            }
        }
    }
    return -1;
}

int isCompleted(int graph[SIZE][SIZE], int size) {
    for(int i = 0; i < size; i++) {
        if (getDegree(graph, size, i) > 0)
            return 0;
    }
    return 1;
}

void removeEdge(int graph[SIZE][SIZE], int root, int eNode) {
    graph[root][eNode] = 0;
    graph[eNode][root] = 0;
}

void eulerFind(int graph[SIZE][SIZE], int finalPath[SIZE], int size, int root) {   
    int eNode;
    while (!isCompleted(graph, size)) {
        eNode = getNextNode(graph, size, root);
        if (eNode == -1) 
            break; // Protección contra nodo no encontrado

        finalPath[fp++] = eNode;
        removeEdge(graph, root, eNode);
        root = eNode;
    }
}

int main() {

    int graph[SIZE][SIZE] = {
        {0, 1, 0, 0, 0},
        {1, 0, 1, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 1, 0, 1},
        {0, 0, 0, 1, 0}
    };

    int finalPath[SIZE];

    int size = 5;

    int root = findRoot(graph, size);
    
    if (root) {
        if (count) 
            printf("Available Euler Path is\n");
        else 
            printf("Available Euler Circuit is\n");

        finalPath[fp++] = root;
        eulerFind(graph, finalPath, size, root);
        displayPath(finalPath);
    } else
        printf("Euler path or circuit not available\n");
    
    return 0;
}