chmod_prog = executable(
    'chmod',
    [ 'chmod.c' ],
    include_directories : [ inc, cinc ],
    link_with : [ libcompat, libutil_static ],
    install : true,
)

install_man('chmod.1')
