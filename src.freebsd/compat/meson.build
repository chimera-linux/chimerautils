libcompat_sources = [
    'getbsize.c',
    'heapsort.c',
    'humanize_number.c',
    'merge.c',
    'mktemp.c',
    'ohash.c',
    'setmode.c',
    'strmode.c',
    'strftime.c',
    'strptime.c',
    'stringlist.c',
    'expand_number.c',
    'unvis.c',
    'vis.c',
    '../../src.compat/b64.c',
    '../../src.compat/err.c',
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
