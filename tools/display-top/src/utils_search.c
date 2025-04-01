#include <stdio.h>
#include <string.h>

#include "node.h"
#include "utils.h"

void searchNodes(Node *root, const char *searchInput, Node *results)
{

    if (root == NULL)
    {
        return;
    }

    if (strstr(root->name, searchInput) != NULL)
    {
        addChild(results, root);
        return;
    }

    for (int i = 0; i < root->childrenSize; i++)
    {
        searchNodes(&root->children[i], searchInput, results);
    }

    return;
}