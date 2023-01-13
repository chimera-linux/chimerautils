cmp_prog = executable(
    'cmp',
    [ 'cmp.c', 'link.c', 'misc.c', 'regular.c', 'special.c', ],
    include_directories : inc,
    link_with : [ libcompat ],
    install : true,
)

install_man('cmp.1')
