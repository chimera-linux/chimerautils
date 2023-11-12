head_prog = executable(
    'head',
    ['head.c'],
    include_directories: inc,
    link_with: [libcompat],
    install: true,
)

install_man('head.1')
