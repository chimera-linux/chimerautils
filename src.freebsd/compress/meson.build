compress_prog = executable(
    'compress',
    ['compress.c', 'zopen.c'],
    include_directories: [inc],
    install: true,
    install_dir: bindir,
)

install_man('compress.1')

install_symlink('uncompress', pointing_to: 'compress', install_dir: bindir)
install_symlink('uncompress.1', pointing_to: 'compress.1', install_dir: man1dir)
