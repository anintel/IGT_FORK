#include "populate.h"
#include "data.h"

void initializeFtrace()
{
    Node *ftrace = createNode("Ftrace", displayFtraceOptions, root);
    if (!ftrace)
    {
        log_message(LOG_ERROR, "Failed to create Ftrace node");
        return;
    }

    Node *LiveTracing = createNode("Live Tracing", displayLiveTracing, ftrace);
    if (!LiveTracing)
    {
        log_message(LOG_ERROR, "Failed to create Live Tracing node");
        return;
    }
    addChild(ftrace, LiveTracing);

    Node *DumpTracing = createNode("Dump Tracing", displayDumpTracing, ftrace);
    if (!DumpTracing)
    {
        log_message(LOG_ERROR, "Failed to create Dump Tracing node");
        return;
    }
    addChild(ftrace, DumpTracing);

    Node *RegRangeTracing = createNode("Filter Tracing", displayFilterTracing, ftrace);
    if (!RegRangeTracing)
    {
        log_message(LOG_ERROR, "Failed to create Register Range Tracing node");
        return;
    }
    addChild(ftrace, RegRangeTracing);

    addChild(root, ftrace);

    return;
}