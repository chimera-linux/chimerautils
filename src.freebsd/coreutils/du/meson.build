du_prog = executable(
    'du',
    ['du.c'],
    dependencies: [libxo],
    include_directories: [inc, cinc],
    link_with: [libcompat, libutil_static],
    install: true,
)

install_man('du.1')
