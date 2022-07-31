wall_prog = executable(
    'wall',
    [ 'wall.c', 'ttymsg.c' ],
    include_directories : inc,
    link_with : [ libcompat ],
    install : true,
)

install_man('wall.1')
