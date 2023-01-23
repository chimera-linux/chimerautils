md5sum = executable('md5sum', ['md5sum.c'],
    dependencies: [libcrypto],
    include_directories: inc_main,
    install: true
)

install_symlink('md5', pointing_to: 'md5sum', install_dir: bindir)
install_symlink('b2sum', pointing_to: 'md5sum', install_dir: bindir)

foreach l: ['sha1', 'sha224', 'sha256', 'sha384', 'sha512']
    install_symlink(l, pointing_to: 'md5sum', install_dir: bindir)
    install_symlink(l + 'sum', pointing_to: 'md5sum', install_dir: bindir)
endforeach
