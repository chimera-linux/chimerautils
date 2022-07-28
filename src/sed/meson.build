sed_prog = executable(
    'sed',
    [ 'compile.c', 'main.c', 'misc.c', 'process.c' ],
    include_directories : [ inc, ],
    install : true,
)

install_man('sed.1')
