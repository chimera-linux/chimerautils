vis_prog = executable(
    'vis',
    ['vis.c', 'foldit.c'],
    include_directories: [inc, cinc],
    link_with: [libcompat],
    install: true,
    install_dir: bindir,
)

install_man('vis.1')
