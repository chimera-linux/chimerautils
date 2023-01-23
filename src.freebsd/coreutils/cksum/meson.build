cksum_prog = executable(
    'cksum',
    ['cksum.c', 'crc.c', 'print.c', 'sum1.c', 'sum2.c', 'crc32.c'],
    c_args: ['-Duint32_t=u_int32_t'],
    include_directories: inc,
    install: true,
)
install_man('cksum.1')

install_symlink('sum', pointing_to: 'cksum', install_dir: bindir)
install_symlink('sum.1', pointing_to: 'cksum.1', install_dir: man1dir)
