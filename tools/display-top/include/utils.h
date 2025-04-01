#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include <math.h>
#include <string.h>

#include <error.h>
#include <errno.h>

#include <fcntl.h>
#include <ctype.h>
#include <limits.h>
#include <libgen.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <dirent.h>
#include <unistd.h>
#include <ncurses.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <pci/pci.h>
#include <xf86drm.h>
#include <drm/drm.h>
#include <xf86drmMode.h>
#include <drm/drm_mode.h>
#include <drm/i915_drm.h>
#include <drm/drm_fourcc.h>

#include <cjson/cJSON.h>

#include "node.h"
#include "log.h"
#include "uthash.h"
#include "display.h"

#define DUMP_DIR "./Dump"

#define DRM_DIR "/dev/dri/"
#define DRM_PRIMARY_PREFIX "card"
#define DRM_RENDER_PREFIX "renderD"

#define TRACE_PATH "/sys/kernel/debug/tracing/trace"

extern int drm_fd;

typedef struct
{
    uint32_t reg_addr;
    char name[64];
    UT_hash_handle hh;
} RegisterCacheEntry;

/* Display utility Functions */
void setString(char *dest, const char *src, size_t size);
int check_size_change(WINDOW *win, int *height, int *width);

void singleDump(Node *node, const char *filePath, bool single);
void searchNodes(Node *root, const char *searchInput, Node *results);

void print_bold_text(WINDOW *win, int line, int col, const char *text, ...);
void print_dim_text(WINDOW *win, int line, int col, const char *text, ...);
void print_red_text(WINDOW *win, int line, int col, const char *text, ...);
void print_green_text(WINDOW *win, int line, int col, const char *text, ...);

void print_wrapped_text(WINDOW *pad, int *line, int start, int size, const char *text, bool enclose_with_pipe);

/* JSON utility Functions */
char *get_file_path(const char *filename);
char *get_mmio_path(Node *node);
char *read_file(const char *filename);
char *getValidValueString(cJSON *validValue);
char *getDescriptionString(cJSON *Description);
uint64_t hex_string_to_address(const char *hex_str);
char *format_bits_from_register(uint32_t value, const char *bits);

void printJson(WINDOW *pad, cJSON *json, int *line, int depth);
void getJsonObject(cJSON *parent, const char *name, cJSON **result);
void displayDescription(WINDOW *pad, cJSON *Description, int *line, int start, int size);

off_t get_mmio_base();
uint64_t getAddress(cJSON *Address);
uint32_t read_mmio_register(uint64_t address);
uint32_t read_register_value(off_t register_offset);

/* DRM utility Functions */
char *find_drm_device(bool primary);
void open_primary_drm_device();
void close_primary_drm_device();
const char *get_drm_object_type_name(uint32_t object_type);
const char *get_format_str(uint32_t format);
const char *get_basic_modifier_str(uint64_t modifier);
const char *get_connector_type_name(uint32_t connector_type);
const char *get_encoder_type_name(uint32_t encoder_type);

void strip_whitespace(char *str);
void EnsureTracingOn();
void add_to_cache(uint32_t reg_addr, const char *name);
const char *lookup_cache(uint32_t reg_addr);
const char *getRegisterNameFromJson(uint32_t reg_addr, cJSON *json);
void free_cache();

void ensureDumpDirectory();

void run_display_animation();

#endif // UTILS_H