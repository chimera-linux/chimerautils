locate_prog = executable(
    'locate',
    ['locate.c', 'util.c'],
    include_directories: inc,
    c_args: ['-DMMAP'],
    install: true,
)

install_man('locate.1')
install_man('locate.updatedb.8')

install_symlink(
    'updatedb.8', pointing_to: 'locate.updatedb.8', install_dir: man8dir
)

configure_file(
    input: 'updatedb.sh.in',
    output: 'locate.updatedb',
    configuration: confd,
    format: 'cmake@',
    install_dir: get_option('libexecdir'),
    install_mode: 'rwxr-xr-x'
)

configure_file(
    input: 'mklocatedb.sh.in',
    output: 'locate.mklocatedb',
    configuration: confd,
    format: 'cmake@',
    install_dir: get_option('libexecdir'),
    install_mode: 'rwxr-xr-x'
)

configure_file(
    input: 'concatdb.sh.in',
    output: 'locate.concatdb',
    configuration: confd,
    format: 'cmake@',
    install_dir: get_option('libexecdir'),
    install_mode: 'rwxr-xr-x'
)

install_data(
    'locate.rc',
    install_dir: get_option('sysconfdir'),
    install_mode: 'rw-r--r--'
)

