#include <form.h>

#include "display.h"
#include "utils.h"

char *getRangeInput()
{
    int rows, cols;
    wclear(stdscr);
    wrefresh(stdscr);
    getmaxyx(stdscr, rows, cols);
    WINDOW *overlay = newwin(rows, cols, 0, 0);
    wbkgd(overlay, COLOR_PAIR(6));
    keypad(overlay, TRUE);
    box(overlay, 0, 0);
    wrefresh(overlay);

    // Instruction Manual
    print_bold_text(overlay, 1, 1, "Trace Log Filtering - Instructions");
    mvwprintw(overlay, 3, 2, "1. Enter a single register address (e.g., 0x40000)");
    mvwprintw(overlay, 4, 2, "2. Enter an address range (e.g., 0x40000-0x40400)");
    mvwprintw(overlay, 5, 2, "3. Press ENTER to apply filter, ESC to exit.");
    mvwhline(overlay, 6, 1, 0, cols - 2);

    FIELD *fields[2];
    fields[0] = new_field(1, cols - 4, 1, 1, 0, 0);
    fields[1] = NULL;

    set_field_back(fields[0], A_UNDERLINE);
    field_opts_off(fields[0], O_AUTOSKIP);

    FORM *form = new_form(fields);
    set_form_win(form, overlay);
    set_form_sub(form, derwin(overlay, 3, cols - 2, 7, 1));

    wbkgd(form_sub(form), COLOR_PAIR(7));
    box(form_sub(form), 0, 0);
    post_form(form);
    wrefresh(overlay);

    char *input_buffer = malloc(256);
    if (!input_buffer)
    {
        log_message(LOG_ERROR, "Memory allocation failed for input buffer");
        return NULL;
    }
    memset(input_buffer, 0, 256);

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
                // Apply filter and dump results
                unpost_form(form);
                free_form(form);
                free_field(fields[0]);
                wclear(overlay);
                wrefresh(overlay);
                delwin(overlay);
                return input_buffer;
            }
            break;
        default:
            if (pos < 256 - 1)
            {
                input_buffer[pos++] = ch;
                input_buffer[pos] = '\0';
                form_driver(form, ch);
            }
            break;
        }
        wrefresh(overlay);
    }

    if (form)
    {
        unpost_form(form);
        free_form(form);
    }
    if (fields[0])
    {
        free_field(fields[0]);
    }
    if (overlay)
    {
        wclear(overlay);
        wrefresh(overlay);
        delwin(overlay);
    }
    if (input_buffer)
    {
        free(input_buffer);
    }
    return NULL;
}

void displayFilterTracing(WINDOW *pad, Node *node, int *content_line)
{
    int line = 0;
    char buffer[512];
    size_t log_count = 0;
    time_t now = time(NULL);
    char dump_file_path[256];
    struct tm *t = localtime(&now);
    FILE *trace_fp = NULL, *dump_fp = NULL;

    strftime(dump_file_path, sizeof(dump_file_path), DUMP_DIR "/filtered_trace_dump_%Y-%m-%d_%H.%M.%S.txt", t);

    *content_line = line;
    line = 3;

    EnsureTracingOn();

    print_bold_text(pad, line++, 1, "%s", node->name);
    line++;

    char *input_buffer = getRangeInput();
    if (!input_buffer || strlen(input_buffer) == 0)
    {
        log_message(LOG_ERROR, "Failed to get range input");
        free(input_buffer);
        return;
    }

    uint32_t start_addr = 0, end_addr = 0;
    if (strchr(input_buffer, '-') != NULL)
    {
        if (sscanf(input_buffer, "0x%x-0x%x", &start_addr, &end_addr) != 2)
        {
            print_bold_text(pad, line++, 2, "Error: Invalid address range! Please retry.");
            free(input_buffer);
            return;
        }
        if (start_addr > end_addr)
        {
            print_bold_text(pad, line++, 2, "Error: Invalid range! Please retry.");
            free(input_buffer);
            return;
        }
    }
    else
    {
        if (sscanf(input_buffer, "0x%x", &start_addr) != 1)
        {
            print_bold_text(pad, line++, 2, "Error: Invalid address! Please retry.");
            free(input_buffer);
            return;
        }
        end_addr = start_addr;
    }

    char json_file_path[] = "./data/Registers.json";
    char *json_data = read_file(json_file_path);
    if (!json_data)
    {
        log_message(LOG_ERROR, "Error reading JSON file");
        goto cleanup;
    }

    cJSON *json = cJSON_Parse(json_data);
    if (!json)
    {
        log_message(LOG_ERROR, "Error parsing JSON");
        goto cleanup;
    }

    trace_fp = fopen(TRACE_PATH, "r");
    if (!trace_fp)
    {
        log_message(LOG_ERROR, "Error opening trace file: %s", strerror(errno));
        goto cleanup;
    }

    int width = getmaxx(pad);
    width -= 9;

    int col_widths[7] = {
        width * 0.10f,
        width * 0.05f,
        width * 0.10f,
        width * 0.08f,
        width * 0.10f,
        width * 0.17f,
        width * 0.40f};

    dump_fp = fopen(dump_file_path, "w");
    if (!dump_fp)
    {
        log_message(LOG_ERROR, "Error creating dump file: %s", strerror(errno));
        goto cleanup;
    }

    wclear(pad);
    wattron(pad, A_BOLD);
    mvwprintw(pad, line++, 1, "| %-*s | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s |",
              col_widths[0], "Process",
              col_widths[1], "CPU",
              col_widths[2], "Timestamp",
              col_widths[3], "R/W",
              col_widths[4], "Reg",
              col_widths[5], "Value",
              col_widths[6], "Name");
    wattroff(pad, A_BOLD);

    int total_lines = 0;
    int processed_lines = 0;
    while (fgets(buffer, sizeof(buffer), trace_fp) != NULL)
    {
        if (strstr(buffer, "i915_reg_rw") != NULL)
        {
            total_lines++;
        }
    }
    rewind(trace_fp);

    // Create a new window for the progress bar
    WINDOW *progress_win = newwin(getmaxy(stdscr), getmaxx(stdscr), 0, 0);
    wbkgd(progress_win, COLOR_PAIR(6));
    wclear(progress_win);
    box(progress_win, 0, 0);
    print_bold_text(progress_win, 1, 1, "FILTERING IS IN PROGRESS :)");
    wrefresh(progress_win);

    while (fgets(buffer, sizeof(buffer), trace_fp) != NULL)
    {
        if (strstr(buffer, "i915_reg_rw") == NULL)
            continue;

        char process[32], op[8];
        int pid, cpu, val1, val2, len;
        uint32_t reg;
        double timestamp;

        if (sscanf(buffer, "%31[^-]-%d  [%d] %*s %lf: i915_reg_rw: %7s reg=0x%x, len=%d, val=(0x%x, 0x%x)",
                   process, &pid, &cpu, &timestamp, op, &reg, &len, &val1, &val2) == 9)
        {
            strip_whitespace(process);
            strip_whitespace(op);

            char value_str[64];
            snprintf(value_str, sizeof(value_str), "(0x%X, 0x%X)", val1, val2);
            const char *reg_name = getRegisterNameFromJson(reg, json);
            if (!reg_name)
            {
                reg_name = "Unknown";
            }

            char formatted_time[64];
            time_t raw_time = (time_t)(timestamp / 1000000);
            struct tm *time_info = localtime(&raw_time);
            strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", time_info);

            if (reg >= start_addr && reg <= end_addr)
            {
                log_count++;

                // write on the screen
                mvwprintw(pad, line++, 1, "| %-*s | %-*d | %-*.2f | %-*s | 0x%-*X | %-*s | %-*s |",
                          col_widths[0], process,
                          col_widths[1], cpu,
                          col_widths[2], timestamp,
                          col_widths[3], op,
                          col_widths[4] - 2, reg,
                          col_widths[5], value_str,
                          col_widths[6], reg_name);

                // dump into the file
                fprintf(dump_fp, "| %-*s | %-*d | %-*.2f | %-*s | 0x%-*X | %-*s | %-*s |\n",
                        col_widths[0], process,
                        col_widths[1], cpu,
                        col_widths[2], timestamp,
                        col_widths[3], op,
                        col_widths[4] - 2, reg,
                        col_widths[5], value_str,
                        col_widths[6], reg_name);
            }
            if (line >= getmaxy(pad) - 2)
            {
                wscrl(pad, 1);
                line--;
            }

            processed_lines++;

            // Update progress bar
            float progress = (float)processed_lines / total_lines;
            int bar_width = getmaxx(progress_win) - 14; // Space for brackets & percentage
            int pos = bar_width * progress;

            wattron(progress_win, A_BOLD);
            mvwprintw(progress_win, 3, 2, "[");
            wattroff(progress_win, A_BOLD);

            for (int i = 0; i < bar_width; i++)
            {
                if (i < pos - 1) // Filled progress
                {
                    wattron(progress_win, COLOR_PAIR(7));
                    waddch(progress_win, ' ');
                    wattroff(progress_win, COLOR_PAIR(7));
                }
                else if (i == pos - 1) // Moving arrow indicator
                {
                    wattron(progress_win, COLOR_PAIR(7) | A_BOLD);
                    waddch(progress_win, ' ');
                    wattroff(progress_win, COLOR_PAIR(7) | A_BOLD);
                }
                else // Empty space
                {
                    wattron(progress_win, COLOR_PAIR(2));
                    waddch(progress_win, ' ');
                    wattroff(progress_win, COLOR_PAIR(2));
                }
            }

            wattron(progress_win, A_BOLD);
            waddch(progress_win, ']');
            wattroff(progress_win, A_BOLD);

            // Display percentage
            wattron(progress_win, A_BOLD);
            mvwprintw(progress_win, 3, bar_width + 4, "%3d%%", (int)(progress * 100));
            wattroff(progress_win, A_BOLD);

            wrefresh(progress_win);
        }
    }

    wattron(pad, A_BOLD | COLOR_PAIR(2));
    mvwprintw(pad, 1, 1, "Filtered range: 0x%X - 0x%X, Hits: %zu", start_addr, end_addr, log_count);
    wattroff(pad, A_BOLD | COLOR_PAIR(2));

    if (log_count == 0)
    {
        remove(dump_file_path);
        mvwprintw(pad, line + 2, 1, "No matching logs found. Dump file not created.");
    }
    else
    {
        mvwprintw(pad, line + 2, 1, "Dump saved to: %s", dump_file_path);
    }

    *content_line = line;

cleanup:
    if (trace_fp)
        fclose(trace_fp);
    if (dump_fp)
        fclose(dump_fp);
    if (json)
        cJSON_Delete(json);
    if (json_data)
        free(json_data);
    if (input_buffer)
        free(input_buffer);

    return;
}
