#include "data.h"
#include "display.h"

static DPMapping *dp_mappings = NULL;
static int dp_mapping_count = 0;

DPMapping **get_dp_mapping_storage()
{
    return &dp_mappings;
}

int *get_dp_mapping_count_ptr()
{
    return &dp_mapping_count;
}

Node *root;