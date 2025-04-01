#include "node.h"

Node* createNode(const char* name, void (*displayFunction)(WINDOW*, Node*, int*), Node* parent)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    strncpy(newNode->name, name, sizeof(newNode->name) - 1);
    newNode->name[sizeof(newNode->name) - 1] = '\0';
    newNode->displayFunction = displayFunction;
    newNode->parent = parent;
    newNode->children = NULL;
    newNode->childrenSize = 0;
    return newNode;
}

void addChild(Node *parent, Node *child)
{
    parent->children = (Node *)realloc(parent->children, sizeof(Node) * (parent->childrenSize + 1));
    parent->children[parent->childrenSize] = *child;
    parent->childrenSize++;
}

void freeTree(Node *root)
{
    if (!root)
        return;
    for (int i = 0; i < root->childrenSize; i++)
    {
        freeTree(&root->children[i]);
    }
    free(root->children);
    free(root);
}

void printPath(Node *node)
{
    if (node->parent)
    {
        printPath(node->parent);
        printw(" > %s", node->name);
    }
    else
    {
        printw("%s", node->name);
    }
}

char *getPath(Node *node)
{
    if (!node)
    {
        return NULL;
    }

    // Determine required buffer size
    size_t path_size = 1; // Null terminator
    Node *current = node;
    while (current)
    {
        path_size += strlen(current->name) + 3; // " > " separator
        current = current->parent;
    }

    // Allocate buffer dynamically
    char *path = (char *)malloc(path_size);
    if (!path)
    {
        return NULL;
    }
    path[0] = '\0'; // Ensure it's an empty string

    // Build path from leaf to root
    current = node;
    while (current)
    {
        // Prepend the name to the path
        char *new_path = (char *)malloc(strlen(current->name) + strlen(path) + 4); // " > " + null terminator
        if (!new_path)
        {
            free(path);
            return NULL;
        }

        if (path[0] == '\0')
        {
            snprintf(new_path, strlen(current->name) + 1, "%s", current->name);
        }
        else
        {
            snprintf(new_path, strlen(current->name) + strlen(path) + 4, "%s > %s", current->name, path);
        }

        free(path);
        path = new_path;
        current = current->parent;
    }

    return path;
}

void traverseAndPrint(Node *node, int depth)
{
    if (node == NULL)
    {
        return;
    }

    for (int i = 0; i < depth; i++)
    {
        printw("-------");
    }

    if (node->parent == NULL)
    {
        printw("Node %s has no parent\n", node->name);
    }
    else
    {
        printw("Node %s is a child of %s\n", node->name, node->parent->name);
    }

    for (int i = 0; i < node->childrenSize; i++)
    {
        traverseAndPrint(&node->children[i], depth + 1);
    }
}

void countNodes(Node *node, int *count)
{
    if (node == NULL)
    {
        return;
    }

    (*count)++;
    for (int i = 0; i < node->childrenSize; i++)
    {
        countNodes(&node->children[i], count);
    }
}