ln_prog = executable(
    'ln',
    [ 'ln.c' ],
    include_directories : inc,
    link_with : [ libcompat ],
    install : true,
)

install_man('ln.1')
install_man('symlink.7')

install_symlink('link', pointing_to: 'ln', install_dir: bindir)
install_symlink('link.1', pointing_to: 'ln.1', install_dir: man1dir)
