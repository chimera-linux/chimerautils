rm_prog = executable(
    'rm',
    [ 'rm.c' ],
    include_directories : [ inc, ],
    dependencies : [ libfts, ],
    link_with : [ libcompat, ],
    install : true,
)

install_man('rm.1')

install_symlink('unlink', pointing_to: 'rm', install_dir: bindir)
install_symlink('unlink.1', pointing_to: 'rm.1', install_dir: man1dir)
