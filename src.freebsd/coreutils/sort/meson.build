sort_cargs = ['-DWITHOUT_NLS', '-DSORT_THREADS']

if not libcrypto.found()
    sort_cargs += ['-DWITHOUT_LIBCRYPTO']
endif

sort_srcs = [
    'bwstring.c', 'coll.c', 'file.c', 'mem.c',
    'radixsort.c', 'sort.c', 'vsort.c',
]

sort_prog = executable(
    'sort', sort_srcs,
    include_directories: [inc],
    dependencies: [libcrypto, libpthread],
    link_with: [libcompat],
    c_args: sort_cargs,
    install: true,
)

if tiny.enabled() and libcrypto.found()
    sortiny_prog = executable(
        'sort.tiny', sort_srcs,
        include_directories: [inc],
        link_with: [libcompat],
        c_args: ['-DWITHOUT_NLS', '-DWITHOUT_LIBCRYPTO'],
        install: true,
    )
endif

install_man('sort.1')
