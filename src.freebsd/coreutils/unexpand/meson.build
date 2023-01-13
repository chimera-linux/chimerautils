unexpand_prog = executable(
    'unexpand',
    [ 'unexpand.c' ],
    include_directories : inc,
    install : true,
)

# references expand in another dir
meson.add_install_script(
    install_link, 'expand.1', 'unexpand.1',
    join_paths(get_option('mandir'), 'man1')
)
