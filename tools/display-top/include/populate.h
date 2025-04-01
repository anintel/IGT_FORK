#ifndef POPULATE_H
#define POPULATE_H

#include "node.h"
#include "data.h"
#include "utils.h"
#include "utils.h"
#include "display.h"

void populateData();

void initializeDisplayConfig();
void initializeDisplayDebugfs();
void initializeMMIO();
void initializeDPCD();
void initializeFtrace();

#endif // POPULATE_H