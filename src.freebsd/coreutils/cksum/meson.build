cksum_prog = executable(
    'cksum',
    ['cksum.c', 'crc.c', 'print.c', 'sum1.c', 'sum2.c', 'crc32.c'],
    c_args: ['-Duint32_t=u_int32_t'],
    include_directories: inc,
    install: true,
)
install_man('cksum.1')

meson.add_install_script(install_link, 'cksum', 'sum', get_option('bindir'))
meson.add_install_script(
    install_link, 'cksum.1', 'sum.1', join_paths(get_option('mandir'), 'man1')
)
