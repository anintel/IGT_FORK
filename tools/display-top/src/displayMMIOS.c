#include "display.h"
#include "utils.h"
#include "utils.h"

#include <dirent.h>
#include <string.h>

void traverseAndPrintCount(Node *node, int *count, WINDOW *pad, int *content_line, int depth)
{
    if (node == NULL)
    {
        return;
    }

    if (node->childrenSize == 0)
    {
        (*count)++;
        return;
    }

    (*count)++;

    wmove(pad, *content_line, 0);
    wattron(pad, A_DIM);
    for (int i = 0; i < depth; i++)
    {
        if (i == depth - 1)
        {
            wprintw(pad, "|--");
        }
        else
        {
            wprintw(pad, "|   ");
        }
    }
    wattroff(pad, A_DIM);

    mvwprintw(pad, (*content_line)++, depth * 4, "%s (%d)", node->name, node->childrenSize);

    for (int i = 0; i < node->childrenSize; i++)
    {
        traverseAndPrintCount(&node->children[i], count, pad, content_line, depth + 1);
    }
}

void displayMMIOSummary(WINDOW *pad, Node *node, int *content_line)
{
    *content_line = 0;
    int node_count = 0;

    if (node == NULL)
    {
        return;
    }

    traverseAndPrintCount(node, &node_count, pad, content_line, 0);

    mvwprintw(pad, 0, 0, "MMIO Registers: %d", node_count);
}