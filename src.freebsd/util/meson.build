libutil_sources = [
    'fts.c',
    'rpmatch.c',
]

util_cargs = []

# make sure to leave out lto so we get an object archive
if cc.has_argument('-flto')
    util_cargs += ['-fno-lto']
    # make sure to leave out cfi so static libs are usable and so
    # that this compiles with lto explicitly left out like above
    # we can't use cc.has_argument because cfi requires the others
    if cc.compiles('int main(void) {}', args: [
        '-flto', '-fvisibility=hidden', '-fsanitize=cfi'
    ])
        util_cargs += ['-fno-sanitize=cfi']
    endif
endif

libutil_static = static_library(
    'util_static',
    libutil_sources,
    include_directories: [inc, cinc],
    c_args: util_cargs,
    link_args: util_cargs,
)

libutil = library(
    'chimerautils',
    version: '0',
    link_whole: [libutil_static],
    install: get_option('install-lib'),
)

lopt = get_option('default_library')
ldir = get_option('libdir')

if not cc.has_function('rpmatch') and get_option('install-lib')
    install_headers('../include/rpmatch.h')
    install_man('rpmatch.3')
    if lopt == 'shared' or lopt == 'both'
        install_symlink('librpmatch.so', pointing_to: 'libchimerautils.so', install_dir: ldir)
    endif
    if opt == 'static' or lopt == 'both'
        install_symlink('librpmatch.a', pointing_to: 'libchimerautils.a', install_dir: ldir)
    endif
endif

if not cc.has_function('fts_open') and get_option('install-lib')
    install_headers('../include/fts.h')
    install_man('fts.3')
    if lopt == 'shared' or lopt == 'both'
        install_symlink('libfts.so', pointing_to: 'libchimerautils.so', install_dir: ldir)
    endif
    if opt == 'static' or lopt == 'both'
        install_symlink('libfts.a', pointing_to: 'libchimerautils.a', install_dir: ldir)
    endif
endif
