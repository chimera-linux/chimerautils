locate_bigram_prog = executable(
    'locate.bigram',
    ['locate.bigram.c'],
    include_directories: [inc, '../locate'],
    #link_with: [libcompat],
    install: true,
    install_dir: get_option('libexecdir'),
)
