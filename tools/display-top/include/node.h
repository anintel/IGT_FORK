#ifndef NODE_H
#define NODE_H

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

/**
 * @struct Node
 * @brief Represents a node in a tree structure.
 *
 * This structure is used to represent a node in a tree, where each node can have a name,
 * a display function, a parent node, and an array of child nodes.
 *
 * @var Node::name
 * A character array to store the name of the node. The name can be up to 49 characters long,
 * with the 50th character reserved for the null terminator.
 *
 * @var Node::displayFunction
 * A pointer to a function that takes a WINDOW pointer and a variable number of arguments,
 * and returns an integer. This function is used to display the node.
 *
 * @var Node::parent
 * A pointer to the parent node. If the node is the root, this will be NULL.
 *
 * @var Node::children
 * A pointer to an array of child nodes. If the node has no children, this will be NULL.
 *
 * @var Node::childrenSize
 * An integer representing the number of children the node has.
 */
typedef struct Node
{
    char name[50];
    void (*displayFunction)(WINDOW *, struct Node *, int *); // Fixed argument list
    struct Node *parent;
    struct Node *children;
    int childrenSize;
} Node;

/**
 * @brief Creates a new node with the given name, display function, and parent.
 *
 * This function allocates memory for a new node, initializes its name, display function,
 * parent, and sets its children to NULL and childrenSize to 0.
 *
 * @param name The name of the node.
 * @param displayFunction A pointer to the display function associated with the node.
 * @param parent A pointer to the parent node.
 * @return A pointer to the newly created node.
 */
Node *createNode(const char *name, void (*displayFunction)(WINDOW *, Node *, int *), Node *parent);

/**
 * @brief Adds a child node to the specified parent node.
 *
 * This function reallocates memory for the parent's children array to accommodate the new child,
 * assigns the child to the next available position in the array, and increments the childrenSize.
 *
 * @param parent A pointer to the parent node.
 * @param child A pointer to the child node to be added.
 */
void addChild(Node *parent, Node *child);

/**
 * @brief Frees the memory allocated for the tree recursively.
 *
 * This function recursively frees the memory allocated for the children of the given root node,
 * then frees the memory allocated for the children array and the root node itself.
 *
 * @param root A pointer to the root node of the tree.
 */
void freeTree(Node *root);

/**
 * @brief Prints the path from the given node.
 *
 * This function takes a pointer to a Node structure and prints the path
 * associated with that node. The exact format and details of the path
 * are dependent on the implementation of the Node structure and the
 * printPath function.
 *
 * @param node A pointer to the Node whose path is to be printed.
 */
void printPath(Node *node);

/**
 * @brief Gets the path from the given node.
 *
 * This function takes a pointer to a Node structure and returns the path
 * associated with that node. The exact format and details of the path
 * are dependent on the implementation of the Node structure and the
 * getPath function.
 *
 * @param node A pointer to the Node whose path is to be returned.
 * @return A string representing the path of the node.
 */
char *getPath(Node *node);

void countNodes(Node *node, int *count);


#endif
