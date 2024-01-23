nc_prog = executable(
    'nc',
    ['netcat.c', 'atomicio.c', 'socks.c'],
    include_directories: [inc],
    link_with: [libcompat],
    install: true,
    install_dir: bindir,
)

install_man('nc.1')
