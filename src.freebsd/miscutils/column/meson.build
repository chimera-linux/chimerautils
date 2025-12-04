column_prog = executable(
    'column',
    ['column.c'],
    include_directories: [inc, cinc],
    link_with: [libcompat],
    install: true,
)

install_man('column.1')
