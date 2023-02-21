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

install_data(
    'updatedb.sh',
    rename: 'locate.updatedb',
    install_dir: get_option('libexecdir'),
    install_mode: 'rwxr-xr-x'
)

install_data(
    'mklocatedb.sh',
    rename: 'locate.mklocatedb',
    install_dir: get_option('libexecdir'),
    install_mode: 'rwxr-xr-x'
)

install_data(
    'concatdb.sh',
    rename: 'locate.concatdb',
    install_dir: get_option('libexecdir'),
    install_mode: 'rwxr-xr-x'
)

install_data(
    'locate.rc',
    install_dir: get_option('sysconfdir'),
    install_mode: 'rw-r--r--'
)

