unvis_prog = executable(
    'unvis',
    ['unvis.c'],
    include_directories: [inc, cinc],
    link_with: [libcompat],
    install: true,
    install_dir: bindir,
)

install_man('unvis.1')
