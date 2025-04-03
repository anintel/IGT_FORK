cflags = ['-Wall', '-Wextra', '-Iinclude', '-I/usr/include/libdrm', '-I/usr/local/include']
cflags += ['-DMESON_SOURCE_ROOT="' + meson.source_root() + '"']
ldflags = ['-ldrm', '-lncurses', '-lmenu', '-lform', '-lpanel', '-lcjson', '-lpciaccess', '-lm', '-lpci']

# Include directories (for header files)
inc_dirs = include_directories('include', '/usr/include/libdrm', '/usr/local/include')

# Dependencies
deps = [
  dependency('libdrm', required: true),
  dependency('ncurses', required: true),
  dependency('pciaccess', required: true),
  dependency('menu', required: false),
  dependency('form', required: false),
  dependency('panel', required: false)
]

# Manually find the pci library if it's not found by pkg-config
pci_dep = cc.find_library('pci', required: true)
cjson_dep = cc.find_library('cjson', required: true)

# Source files (adjust these to match your actual source files)
source = files(
    'src/animation.c',
    'src/data.c',
    'src/display.c',
    'src/displayConfiguration.c',
    'src/displayDebugfs.c',
    'src/displayDPCD.c',
    'src/displayDump.c',
    'src/displayDumpTracing.c',
    'src/displayFilterTracing.c',
    'src/displayFtrace.c',
    'src/displayLiveTracing.c',
    'src/displayMMIO.c',
    'src/displayMMIOS.c',
    'src/displaySearchBar.c',
    'src/displaySummary.c',
    'src/log.c',
    'src/main.c',
    'src/node.c',
    'src/populate.c',
    'src/populateDisplayConfig.c',
    'src/populateDisplayDebugfs.c',
    'src/populateDPCD.c',
    'src/populateFtrace.c',
    'src/populateMMIO.c',
    'src/reg_read.c',
    'src/utils_display.c',
    'src/utils_drm.c',
    'src/utils_json.c',
    'src/utils_search.c',
    'src/utils_trace.c',
    'src/dummy.c'
)

# Build executable
executable('displaytop', source,
  install: true,
  c_args: cflags,
  include_directories: inc_dirs,  # Add this to specify include directories
  link_args: ldflags,
  dependencies: deps + [cjson_dep, pci_dep]
)
