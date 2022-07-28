grep_prog = executable(
    'grep',
    [ 'grep.c', 'file.c', 'queue.c', 'util.c' ],
    include_directories : [ inc, ],
    dependencies : [ libfts, ],
    install : true,
)

install_data(
    'zgrep.sh',
    install_dir : get_option('bindir'),
    install_mode : 'rwxr-xr-x',
    rename : 'zgrep',
)

install_man('grep.1')
install_man('zgrep.1')

foreach l: ['egrep', 'fgrep', 'rgrep']
    meson.add_install_script(install_link, 'grep', l, get_option('bindir'))
    meson.add_install_script(
        install_link, 'grep.1', l + '.1',
        join_paths(get_option('mandir'), 'man1')
    )
endforeach

foreach l: [
    'zfgrep', 'zegrep', 'bzgrep', 'bzegrep', 'bzfgrep',
    'lzgrep', 'lzegrep', 'lzfgrep', 'xzgrep', 'xzegrep', 'xzfgrep',
    'zstdgrep', 'zstdegrep', 'zstdfgrep'
]
    meson.add_install_script(install_link, 'zgrep', l, get_option('bindir'))
    meson.add_install_script(
        install_link, 'zgrep.1', l + '.1',
        join_paths(get_option('mandir'), 'man1')
    )
endforeach
