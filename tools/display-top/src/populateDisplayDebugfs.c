#include "populate.h"
#include "utils.h"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ncurses.h>

#define DEBUGFS_DRI_PATH "/sys/kernel/debug/dri"

char *find_debugfs_dir()
{
    DIR *dir = opendir(DEBUGFS_DRI_PATH);
    if (!dir)
    {
        log_message(LOG_ERROR, "Failed to open debugfs dri directory");
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {

        if (!isdigit(entry->d_name[0]))
            continue;

        char *selectedPath = malloc(512);
        if (!selectedPath)
        {
            log_message(LOG_ERROR, "Memory allocation failed");
            closedir(dir);
            return NULL;
        }

        snprintf(selectedPath, 512, "%s/%s", DEBUGFS_DRI_PATH, entry->d_name);
        closedir(dir);
        return selectedPath;
    }

    closedir(dir);
    return NULL;
}

void populateDebugfs(Node *root)
{
    char *selectedPath = find_debugfs_dir();
    if (!selectedPath)
    {
        return;
    }

    DIR *dir = opendir(selectedPath);
    if (!dir)
    {
        log_message(LOG_ERROR, "Failed to open debugfs directory");
        free(selectedPath);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char fullPath[512];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", selectedPath, entry->d_name);

        struct stat pathStat;
        stat(fullPath, &pathStat);

        if (S_ISREG(pathStat.st_mode))
        {
            Node *newNode = createNode(entry->d_name, displayDebugfsFile, root);
            addChild(root, newNode);
        }
    }
    closedir(dir);
    free(selectedPath);
}



void initializeDisplayDebugfs()
{
    Node *displayDebugfs = createNode("Display Debugfs", NULL, root);
    populateDebugfs(displayDebugfs);
    if(displayDebugfs->childrenSize == 0)
    {
        free(displayDebugfs);
        return;
    }
    addChild(root, displayDebugfs);
}
