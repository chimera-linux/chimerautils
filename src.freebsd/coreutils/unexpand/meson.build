unexpand_prog = executable(
    'unexpand',
    [ 'unexpand.c' ],
    include_directories : inc,
    install : true,
)

# references expand in another dir
install_symlink('unexpand.1', pointing_to: 'expand.1', install_dir: man1dir)
