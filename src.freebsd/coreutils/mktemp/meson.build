mktemp_prog = executable(
    'mktemp',
    ['mktemp.c'],
    link_with: [libcompat],
    install: true,
    include_directories: inc,
)

install_man('mktemp.1')
