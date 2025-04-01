#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <sys/stat.h>

#include "node.h"
#include "utils.h"

void singleDump(Node *node, const char *filePath, bool single)
{
    if (node == NULL || node->name[0] == '\0' || node->displayFunction == NULL)
    {
        log_message(LOG_ERROR, "Invalid node to dump");
        return;
    }

    char dumpFilePath[512];
    if (single)
    {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        snprintf(dumpFilePath, sizeof(dumpFilePath), "%s/%s_%04d-%02d-%02d-%02d.%02d.%02d.txt",
                 filePath, node->name, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                 t->tm_hour, t->tm_min, t->tm_sec);
    }
    else
    {
        snprintf(dumpFilePath, sizeof(dumpFilePath), "%s/%s.txt", filePath, node->name);
    }
    FILE *file = fopen(dumpFilePath, "w");
    if (!file)
    {
        log_message(LOG_ERROR, "Failed to open file %s", dumpFilePath);
        return;
    }

    int padHeight = 1000;
    int padWidth = 200;
    WINDOW *virtualWin = newwin(padHeight, padWidth, 0, 0);
    if (!virtualWin)
    {
        log_message(LOG_ERROR, "Failed to create virtual window");
        fclose(file);
        return;
    }

    int line = 0;
    node->displayFunction(virtualWin, node, &line);

    char buffer[padWidth + 1];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(file, "Dump Time: %04d-%02d-%02d %02d:%02d:%02d\n\n",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

    for (int y = 0; y < line; y++)
    {
        wmove(virtualWin, y, 0);
        winnstr(virtualWin, buffer, padWidth);
        fprintf(file, "%s\n", buffer);
    }

    log_message(LOG_INFO, "Dumped %s to %s", node->name, dumpFilePath);
    fclose(file);
    delwin(virtualWin);
}

void recursiveDump(Node *node, const char *parentDir, WINDOW *pad, int *line)
{
    if (node == NULL)
        return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char nodeDir[512];
    if (node->childrenSize > 0)
    {
        snprintf(nodeDir, sizeof(nodeDir), "%s/%s(%04d-%02d-%02d-%02d.%02d.%02d)",
                 parentDir, node->name, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                 t->tm_hour, t->tm_min, t->tm_sec);

        if (mkdir(nodeDir, 0777) == -1 && errno != EEXIST)
        {
            log_message(LOG_ERROR, "Error creating directory %s: %s", nodeDir, strerror(errno));
            return;
        }
        (*line)++;

        int maxPadHeight = getmaxy(pad);
        int viewableHeight = maxPadHeight - 3;
        int padPos = (*line > viewableHeight) ? (*line - viewableHeight) : 0;

        mvwprintw(pad, (*line), 0, "[DUMPED] -> %s", getPath(node));
        prefresh(pad, padPos, 0, 4, 2, maxPadHeight - 3, getmaxx(pad) - 3);
        wrefresh(pad);
    }
    else
    {
        snprintf(nodeDir, sizeof(nodeDir), "%s", parentDir);
    }

    if (node->displayFunction != NULL)
    {
        singleDump(node, nodeDir, false);
        log_message(LOG_INFO, "Dumped %s", node->name);
    }

    for (int i = 0; i < node->childrenSize; i++)
    {
        log_message(LOG_INFO, "Dumping %s", node->children[i].name);
        recursiveDump(&(node->children[i]), nodeDir, pad, line);
    }
}

void displayDumpMenu(Node *head)
{
    if (head == NULL)
    {
        log_message(LOG_ERROR, "head node is NULL, cannot dump");
        return;
    }

    log_message(LOG_INFO, "Starting dump from %s", head->name);

    if (mkdir(DUMP_DIR, 0777) == -1 && errno != EEXIST)
    {
        log_message(LOG_ERROR, "Failed to create Dump directory");
        return;
    }

    int winHeight, winWidth;
    getmaxyx(stdscr, winHeight, winWidth);

    WINDOW *win = newwin(winHeight, winWidth, 0, 0);
    wbkgd(win, COLOR_PAIR(2));
    box(win, 0, 0);
    wattron(win, A_BOLD);
    mvwprintw(win, 1, 1, "DUMP MENU");
    mvwprintw(win, 1, winWidth - strlen("ESC to exit DUMP menu") - 2, "ESC to exit DUMP menu");
    wattroff(win, A_BOLD);
    mvwhline(win, 2, 1, ACS_HLINE, winWidth - 2);
    wrefresh(win);

    int padHeight = 1000;
    int padWidth = winWidth - 2;
    int viewablePadHeight = winHeight - 6;

    WINDOW *pad = newpad(padHeight, padWidth);
    wbkgd(pad, COLOR_PAIR(2));

    int line = 0;
    recursiveDump(head, DUMP_DIR, pad, &line);

    int padPos = 0;
    int ch;

    while ((ch = getch()) != 27)
    {
        wattron(win, A_BOLD);
        mvwprintw(win, 1, 1, "DUMP MENU");
        mvwprintw(win, 1, winWidth - strlen("ESC to exit DUMP menu") - 2, "ESC to exit DUMP menu");
        wattroff(win, A_BOLD);
        mvwhline(win, 2, 1, ACS_HLINE, winWidth - 2);

        if (padPos > 0)
            print_bold_text(win, 3, 2, "...");
        else
            print_bold_text(win, 3, 2, "   ");

        if (padPos < line - viewablePadHeight)
            print_bold_text(win, winHeight - 2, 2, "...");
        else
            print_bold_text(win, winHeight - 2, 2, "   ");
        wrefresh(win);

        switch (ch)
        {
        case KEY_UP:
            if (padPos > 0)
                padPos--;
            break;
        case KEY_DOWN:
            if (padPos < line - viewablePadHeight)
                padPos++;
            break;
        default:
            break;
        }

        prefresh(pad, padPos, 0, 4, 2, winHeight - 3, winWidth - 3);
    }

    delwin(pad);
    delwin(win);
    endwin();
}