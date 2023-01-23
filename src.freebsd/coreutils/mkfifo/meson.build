mkfifo_prog = executable(
    'mkfifo',
    [ 'mkfifo.c' ],
    include_directories : [ inc, ],
    link_with : [ libcompat, ],
    install : true,
    install_dir : bindir,
)

install_man('mkfifo.1')
