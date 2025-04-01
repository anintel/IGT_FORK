#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <ncurses.h>

#include "display.h"
#include "utils.h"

#define DRI_DEVICE "0000:00:02.0"

void displayDebugfsFile(WINDOW *pad, Node *node, int *content_line)
{
    int line = 0;
    int max_x = getmaxx(pad) - 4;

    char full_file_path[256];
    snprintf(full_file_path, sizeof(full_file_path), "/sys/kernel/debug/dri/%s/%s", DRI_DEVICE, node->name);

    FILE *file = fopen(full_file_path, "r");
    if (!file)
    {
        print_red_text(pad, line++, 1, "Error opening file: %s", full_file_path);
        print_red_text(pad, line++, 1, "Make sure the file exists and you have the necessary permissions (sudo).");
        log_message(LOG_ERROR, "Error opening file: %s", full_file_path);
        *content_line = line;
        return;
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file))
    {
        buffer[strcspn(buffer, "\n")] = 0;

        int start = 0;
        while (start < (int)strlen(buffer))  // Cast strlen(buffer) to int
        {

            int break_point = start + max_x;

            if (break_point < (int)strlen(buffer))  // Cast strlen(buffer) to int
            {

                while (break_point > start && !isspace(buffer[break_point]))
                {
                    break_point--;
                }

                if (break_point == start)
                {
                    break_point = start + max_x;
                }
            }

            char temp[break_point - start + 1];
            strncpy(temp, buffer + start, break_point - start);
            temp[break_point - start] = '\0';

            mvwprintw(pad, line, 2, "%s", temp);
            start = (buffer[break_point] == ' ') ? break_point + 1 : break_point;

            line++;
        }
    }

    line++;
    mvwprintw(pad, line++, 2, "-----------------EOF-------------------");

    *content_line = line;
    fclose(file);
}