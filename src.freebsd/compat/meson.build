libcompat_sources = [
    'getbsize.c',
    'heapsort.c',
    'humanize_number.c',
    'merge.c',
    'ohash.c',
    'setmode.c',
    'strmode.c',
    'expand_number.c',
    'vis.c',
    '../../src.compat/strlfuncs.c',
    '../../src.compat/signames.c',
    '../../src.compat/strtonum.c',
    '../../src.compat/reallocf.c',
]

libcompat = static_library(
    'compat',
    libcompat_sources,
    include_directories: [inc, cinc],
    c_args: ['-DHAVE_NBTOOL_CONFIG_H'],
)
