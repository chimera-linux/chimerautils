sdiff_prog = executable(
    'sdiff',
    [ 'sdiff.c', 'edit.c', ],
    include_directories : inc,
    link_with : [ libcompat ],
    install : true,
)

install_man('sdiff.1')
