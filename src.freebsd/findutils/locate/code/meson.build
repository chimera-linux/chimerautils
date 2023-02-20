locate_code_prog = executable(
    'locate.code',
    ['locate.code.c'],
    include_directories: [inc, '../locate'],
    #link_with: [libcompat],
    install: true,
    install_dir: get_option('libexecdir'),
)
