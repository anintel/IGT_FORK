#include <ncurses.h>
#include <form.h>

#include "display.h"
#include "utils.h"

/**
 * @brief Constructs a search path from a given node to the head node.
 *
 * This function constructs a search path string from the given node to the head node.
 * The path is constructed by traversing from the given node up to the head node,
 * appending each node's name to the path string, separated by " > ".
 *
 * @param node Pointer to the starting node.
 * @param head Pointer to the head node until which path is required.
 * @return A dynamically allocated string representing the search path.
 *         The caller is responsible for freeing the allocated memory.
 *         Returns NULL if either node or head is NULL.
 */
char *getSearchPath(Node *node, Node *head)
{
    if (node == NULL || head == NULL)
        return NULL;

    char buffer[1024];
    buffer[0] = '\0';

    Node *current = node;
    while (current && strcmp(current->name, head->name) != 0)
    {
        char temp[1024];
        if (snprintf(temp, sizeof(temp), "%s%s%s", current->name, buffer[0] ? " > " : "", buffer) >= (int)sizeof(temp))
        {
            log_message(LOG_ERROR, "Path string truncated: %s%s%s", current->name, buffer[0] ? " > " : "", buffer);
            return NULL;
        }
        strncpy(buffer, temp, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        current = current->parent;
    }

    char *path = (char *)malloc(strlen(buffer) + 1);
    if (path)
    {
        strcpy(path, buffer);
    }

    return path;
}

/**
 * @brief Displays the children of a given node in a new window.
 *
 * This function displays the children of a given node in a new window.
 * The user can navigate through the children using the arrow keys and select a child by pressing Enter.
 * The function returns the selected child node.
 *
 * @param node Pointer to the node whose children are to be displayed.
 * @param head Pointer to the head node (current location in DisplayTop).
 * @return Pointer to the selected child node.
 *         Returns NULL if either node or node->children is NULL.
 */
Node *displayChildren(Node *node, Node *head)
{
    if (node == NULL || node->children == NULL)
        return NULL;

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int line = 0;
    int pad_pos = 0;
    int highlighted_index = 0; // Removed unused variable 'pad_height'
    // Removed unused variable 'current'

    WINDOW *pad = newpad(100, cols - 2);
    if (pad == NULL)
        return NULL;

    wbkgd(pad, COLOR_PAIR(6));

    if (node->childrenSize > 0)
    {
        line++;

        for (int i = 0; i < node->childrenSize; ++i)
        {
            if (highlighted_index == i)
                wattron(pad, A_REVERSE);

            mvwprintw(pad, line + i, 1, "%d. %s - %s", i + 1, node->children[i].name, getSearchPath(&node->children[i], head));
            wattroff(pad, A_REVERSE);
        }
        prefresh(pad, pad_pos, 0, 8, 1, rows - 2, cols - 2);
    }

    int ch;
    while ((ch = getch()) != 27)
    {
        if (pad_pos > 0)
            mvwprintw(pad, 0, cols - 5, "...");
        else
            mvwprintw(pad, 0, cols - 5, "   ");

        if (pad_pos < node->childrenSize - (rows - 8))
            mvwprintw(pad, rows - 1, cols - 5, "...");
        else
            mvwprintw(pad, rows - 1, cols - 5, "   ");

        switch (ch)
        {
        case KEY_UP:
            if (highlighted_index > 0)
            {
                highlighted_index--;
                pad_pos = (highlighted_index < pad_pos) ? highlighted_index : pad_pos;
            }
            break;
        case KEY_DOWN:
            if (highlighted_index < node->childrenSize - 1)
            {
                highlighted_index++;
                pad_pos = (highlighted_index >= pad_pos + rows - 8) ? highlighted_index - rows + 8 : pad_pos;
            }
            break;
        case '\n':
            delwin(pad);
            return &node->children[highlighted_index];
        default:
            break;
        }

        for (int i = 0; i < node->childrenSize; ++i)
        {
            if (highlighted_index == i)
                wattron(pad, A_REVERSE);

            mvwprintw(pad, line + i, 1, "%d. %s - %s", i + 1, node->children[i].name, getSearchPath(&node->children[i], head));
            wattroff(pad, A_REVERSE);
        }

        prefresh(pad, pad_pos, 0, 8, 1, rows - 2, cols - 2);
    }

    delwin(pad);
    return head;
}

Node *displaySearchBar(Node *head)
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    WINDOW *win = newwin(rows, cols, 0, 0);
    wbkgd(win, COLOR_PAIR(6));
    keypad(win, TRUE);
    box(win, 0, 0);

    print_bold_text(win, 1, 1, "Search Menu");
    mvwprintw(win, 1, (cols - strlen("Searching under: %s")) / 2, "Searching under: %s", head->name);
    mvwprintw(win, 1, cols - strlen("Press 'Esc' to exit ") - 1, "Press 'Esc' to exit");
    mvwhline(win, 2, 1, 0, cols - 2);
    mvwhline(win, 6, 1, 0, cols - 2);

    FIELD *fields[2];
    fields[0] = new_field(1, cols - 4, 1, 1, 0, 0);
    fields[1] = NULL;

    set_field_back(fields[0], A_UNDERLINE);
    field_opts_off(fields[0], O_AUTOSKIP);

    FORM *form = new_form(fields);
    set_form_win(form, win);
    set_form_sub(form, derwin(win, 3, cols - 2, 3, 1));

    wbkgd(form_sub(form), COLOR_PAIR(7));
    box(form_sub(form), 0, 0);
    post_form(form);
    wrefresh(win);

    char input_buffer[256] = {0};

    int ch, pos = 0;
    while ((ch = getch()) != 27)
    {
        switch (ch)
        {
        case KEY_BACKSPACE:
        case 127:
        case 8:
            if (pos > 0)
            {
                pos--;
                input_buffer[pos] = '\0';
                form_driver(form, REQ_DEL_PREV);
            }
            break;
        case '\n':
            if (pos > 0)
            {
                Node *results = createNode("Search Results", NULL, NULL);
                searchNodes(head, input_buffer, results);

                if (results->childrenSize != 0)
                {
                    unpost_form(form);
                    print_bold_text(win, 7, 2, "%d hits", results->childrenSize);
                    wrefresh(win);
                    Node *result = displayChildren(results, head);
                    free_form(form);
                    free_field(fields[0]);
                    delwin(win);
                    endwin();
                    return result;
                }
                else
                {
                    print_bold_text(win, 7, 2, "No results found");
                }
            }
            break;
        default:
            if (pos < (int)(sizeof(input_buffer) - 1)) // Cast sizeof to int for comparison
            {
                input_buffer[pos++] = ch;
                input_buffer[pos] = '\0';
                form_driver(form, ch);
            }
            break;
        }
        wrefresh(win);
    }

    unpost_form(form);
    wrefresh(win);

    getch();
    free_form(form);
    free_field(fields[0]);
    delwin(win);
    endwin();

    return head;
}
