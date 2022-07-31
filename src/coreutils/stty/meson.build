stty_prog = executable(
    'stty',
    [ 'cchar.c',
      'gfmt.c',
      'key.c',
      'modes.c',
      'print.c',
      'stty.c',
      'util.c',
    ],
    include_directories : inc,
    link_with : [ libcompat ],
    install : true,
)

install_man('stty.1')
