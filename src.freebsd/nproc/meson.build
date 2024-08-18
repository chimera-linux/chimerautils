nproc_prog = executable(
    'nproc', ['nproc.c'],
    include_directories: inc,
    install: true,
    link_with: [libcompat],
)

install_man('nproc.1')
