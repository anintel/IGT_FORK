#include "data.h"
#include "populate.h"

#define DRM_DEVICE "/dev/dri/card0"

void populateData()
{
    root = createNode("Display Top", displaySummary, NULL);

    initializeDisplayConfig();
    initializeDisplayDebugfs();
    initializeMMIO();
    initializeDPCD();
    initializeFtrace();
}
