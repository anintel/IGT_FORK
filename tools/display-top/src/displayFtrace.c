

#include "node.h"
#include "utils.h"
#include "display.h"

void displayFtraceOptions(WINDOW *pad, Node *node, int *content_line)
{

    int line = 0;
    int size = getmaxx(pad) - 2;

    print_bold_text(pad, line++, 1, "%s Menu", node->name);
    line++;

    print_bold_text(pad, line++, 1, "1. Live Tracing");
    wattron(pad, A_DIM);
    print_wrapped_text(pad, &line, 1, size, "Formats and displays the LAST 1000 lines of the ftrace logs, refresh (key r) to see the changes updated", false);
    wattroff(pad, A_DIM);
    line++;

    print_bold_text(pad, line++, 1, "2. Dump Tracing");
    wattron(pad, A_DIM);
    print_wrapped_text(pad, &line, 1, size, "Formats and displays the FIRST 1000 lines but also dumps the entire formatted ", false);
    wattroff(pad, A_DIM);
    line++;

    print_bold_text(pad, line++, 1, "3. Register Range Tracing");
    wattron(pad, A_DIM);
    print_wrapped_text(pad, &line, 1, size, "Filters & formats your desired register range, displays the first 1000 lines in Display Top. FILTERED RESULTS ARE DUMPED BY DEFAULT", false);
    wattroff(pad, A_DIM);
    line++;

    *content_line = line;
}
