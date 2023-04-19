cp_prog = executable(
    'cp',
    ['cp.c', 'utils.c'],
    dependencies: [libfts, libacl],
    include_directories: inc,
    link_with: [libcompat],
    install: true,
)

install_man('cp.1')
