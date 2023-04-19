ls_cargs = []
ls_deps = [libfts, libacl]

if get_option('color_ls')
    ls_cargs += ['-DCOLORLS']
    ls_deps += [tinfo]
    if not tinfo.found()
        error('terminfo library is needed for color ls')
    endif
endif

ls_prog = executable(
    'ls', ['cmp.c', 'ls.c', 'print.c', 'util.c'],
    c_args: ls_cargs,
    include_directories: [inc],
    link_with: [libcompat],
    dependencies: ls_deps,
    install: true,
)

if tiny.enabled() and get_option('color_ls')
    lstiny_prog = executable(
        'ls.tiny', ['cmp.c', 'ls.c', 'print.c', 'util.c'],
        include_directories: [inc],
        link_with: [libcompat],
        dependencies: [libfts],
        install: true
    )
endif

install_man('ls.1')
