# Architecture Documentation

## Overview

This document provides an overview of the architecture of the tool, including how to create and manage nodes in a tree structure. It also covers how users can add more menus, the initialization process, and potential drawbacks.

## Everything is a Node

Display Top is constructed as a tree, where each menu is a node. Each node has a certain set of properties that allow it to be scalable and reusable.

### Node Structure

```c
typedef struct Node
{
    char name[50];
    void (*displayFunction)(WINDOW *, struct Node *, int *); 
    struct Node *parent;
    struct Node *children;
    int childrenSize;
} Node;
```

Reference: [node.h](../include/node.h)

#### **Parameters**

- **name**: Stores the name of the node.
- **displayFunction**:
    - **Type**: `void (*)(WINDOW *, struct Node *, int *)`
    - **Description**: A function pointer that points to a function taking three parameters:
        - `WINDOW *`: A pointer to a WINDOW object, likely used for displaying the node in a graphical or text-based interface.
        - `struct Node *`: A pointer to the current node, allowing access to the node's properties.
        - `int *`: A pointer to an integer, used to keep track of the number of lines of information written into the window.
    - **Importance**: Defines how the node will be rendered or displayed, making the node highly customizable and reusable, as different nodes can have different display behaviors.
    - Can be NULL if there is no need to display anything.
- **parent**:
    - Only the root node has a NULL parent.
- **children**:
    - Pointer to the first child in the array of the node's children.
    - Can be NULL.
- **childrenSize**: 
    - Tracks the number of children because `children` is an array of Node.

#### Summary

The Node structure represents elements in a tree-like hierarchy, with each node having a name, a customizable display function, and pointers to its parent and children. The `displayFunction` is particularly important as it dictates how the node is rendered, making the structure flexible and reusable.
