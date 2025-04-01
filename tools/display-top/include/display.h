#ifndef DISPLAY_H
#define DISPLAY_H

#include "log.h"
#include "node.h"
#include "utils.h"


void displayWin(WINDOW *win, Node *node);

Node *displaySearchBar(Node *head);
void displayDumpMenu(Node *head);

void displayCrtc(WINDOW *pad, Node *node, int *content_line);
void displayPlane(WINDOW *pad, Node *node, int *content_line);
void displayEncoder(WINDOW *pad, Node *node, int *content_line);
void displayConnector(WINDOW *pad, Node *node, int *content_line);

void displaySummary(WINDOW *pad, Node *node, int *content_line);
void displayFormats(WINDOW *pad, Node *node, int *content_line);
void displayInformats(WINDOW *pad, Node *node, int *content_line);
void displayFramebuffer(WINDOW *pad, Node *node, int *content_line);

void displayDebugfsFile(WINDOW *pad, Node *node, int *content_line);

void displayMMIO(WINDOW *pad, Node *node, int *content_line);
void displayMMIOSummary(WINDOW *pad, Node *node, int *content_line);

void displayDPCD(WINDOW *pad, Node *node, int *content_line);
void displayLiveTracing(WINDOW *pad, Node *node, int *content_line);

void displayFtraceOptions(WINDOW *pad, Node *node, int *content_line);
void displayDumpTracing(WINDOW *pad, Node *node, int *content_line);
void displayFilterTracing(WINDOW *pad, Node *node, int *content_line);

#endif