ed_prog = executable(
    'ed',
    [ 'buf.c', 'glbl.c', 'io.c', 'main.c', 're.c', 'sub.c', 'undo.c' ],
    include_directories : [ inc, ],
    link_with : [ libcompat ],
    install : true,
)

install_man('ed.1')
install_symlink('red', pointing_to: 'ed', install_dir: bindir)
install_symlink('red.1', pointing_to: 'ed.1', install_dir: man1dir)
