#ifndef DATA_H
#define DATA_H

#include "node.h"

typedef struct
{
    char *card_name;
    char *dp_name;
} DPMapping;

DPMapping **get_dp_mapping_storage();
int *get_dp_mapping_count_ptr();

/*
 * The root node of the tree
 */
extern Node *root;

// Nodes for the display configuration
extern Node *crtcPages;
extern Node *connectorPages;
extern Node *encoderPages;
extern Node *framebufferPages;
extern Node *planePages;

extern Node *displayConfig;
extern Node *gpuInfoMenu;
extern Node *powerManagementMenu;

extern Node *displayDebugfs;
extern Node *mainMenu;

extern Node displayConfigPages[];
extern Node gpuInfoPages[];
extern Node powerManagementPages[];
extern Node displayCapabilitiesPages[];
extern Node performanceTuningPages[];
extern Node debuggingErrorInfoPages[];
extern Node driverModuleInfoPages[];
extern Node internalClientInfoPages[];
extern Node systemStatePages[];

extern size_t mainMenuSize;
extern size_t gpuInfoPagesSize;
extern size_t powerManagementPagesSize;
extern size_t displayCapabilitiesPagesSize;
extern size_t performanceTuningPagesSize;
extern size_t debuggingErrorInfoPagesSize;
extern size_t driverModuleInfoPagesSize;
extern size_t internalClientInfoPagesSize;
extern size_t systemStatePagesSize;

#endif