cp_prog = executable(
    'cp',
    ['cp.c', 'utils.c'],
    dependencies: [libacl],
    include_directories: [inc, cinc],
    link_with: [libcompat, libutil_static],
    install: true,
)

install_man('cp.1')
