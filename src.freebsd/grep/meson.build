grep_prog = executable(
    'grep',
    [ 'grep.c', 'file.c', 'queue.c', 'util.c' ],
    include_directories : [inc, cinc],
    link_with : [libcompat, libutil_static],
    install : true,
)

install_data(
    'zgrep.sh',
    install_dir : bindir,
    install_mode : 'rwxr-xr-x',
    rename : 'zgrep',
)

install_man('grep.1')
install_man('zgrep.1')

foreach l: ['egrep', 'fgrep', 'rgrep']
    install_symlink(l, pointing_to: 'grep', install_dir: bindir)
    install_symlink(l + '.1', pointing_to: 'grep.1', install_dir: man1dir)
endforeach

foreach l: [
    'zfgrep', 'zegrep', 'bzgrep', 'bzegrep', 'bzfgrep',
    'lzgrep', 'lzegrep', 'lzfgrep', 'xzgrep', 'xzegrep', 'xzfgrep',
    'zstdgrep', 'zstdegrep', 'zstdfgrep'
]
    install_symlink(l, pointing_to: 'zgrep', install_dir: bindir)
    install_symlink(l + '.1', pointing_to: 'zgrep.1', install_dir: man1dir)
endforeach
