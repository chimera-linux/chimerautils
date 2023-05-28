md5_prog = executable(
    'md5',
    ['md5.c'],
    include_directories: inc,
    dependencies: [libcrypto],
    install: true,
)

install_man('md5.1')

install_symlink('md5sum', pointing_to: 'md5', install_dir: bindir)
install_symlink('md5sum.1', pointing_to: 'md5.1', install_dir: man1dir)

foreach l: [
    'sha1', 'sha224', 'sha256', 'sha384', 'sha512', 'sha512t224', 'sha512t256'
]
    install_symlink(l, pointing_to: 'md5', install_dir: bindir)
    install_symlink(l + 'sum', pointing_to: 'md5', install_dir: bindir)
    install_symlink(l + '.1', pointing_to: 'md5.1', install_dir: man1dir)
    install_symlink(l + 'sum.1', pointing_to: 'md5.1', install_dir: man1dir)
endforeach

