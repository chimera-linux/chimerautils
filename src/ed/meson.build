ed_prog = executable(
    'ed',
    [ 'buf.c', 'glbl.c', 'io.c', 'main.c', 're.c', 'sub.c', 'undo.c' ],
    include_directories : [ inc, ],
    link_with : [ libcompat ],
    install : true,
)

install_man('ed.1')

meson.add_install_script(install_link, 'ed', 'red', get_option('bindir'))
meson.add_install_script(
    install_link, 'ed.1', 'red.1', join_paths(get_option('mandir'), 'man1')
)
