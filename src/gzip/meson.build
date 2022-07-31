extra_defs = []
diff_links = [ 'zcmp', 'lzcmp', 'lzdiff' ]

if not bzip2.found()
    extra_defs += [ '-DNO_BZIP2_SUPPORT' ]
else
    diff_links += [ 'bzcmp', 'bzdiff' ]
endif

if not lzma.found()
    extra_defs += [ '-DNO_XZ_SUPPORT' ]
else
    diff_links += [ 'xzcmp', 'xzdiff' ]
endif

gzip_prog = executable(
    'gzip',
    [ 'gzip.c', ],
    include_directories : [ inc, ],
    link_with : [ libcompat ],
    dependencies : [ zlib, bzip2, lzma, libfts, ],
    c_args : extra_defs,
    install : true,
)

install_man('gzip.1')

foreach s: [ 'gzexe', 'zdiff', 'zforce', 'zmore', 'znew' ]
    install_data(
        s,
        install_dir : get_option('bindir'),
        install_mode : 'rwxr-xr-x',
    )
    install_man(s + '.1')
endforeach

foreach l: [ 'gunzip', 'gzcat', 'zcat' ]
    meson.add_install_script(install_link, 'gzip', l, get_option('bindir'))
    meson.add_install_script(
        install_link, 'gzip.1', l + '.1',
        join_paths(get_option('mandir'), 'man1')
    )
endforeach

foreach l: diff_links
    meson.add_install_script(install_link, 'zdiff', l, get_option('bindir'))
    meson.add_install_script(
        install_link, 'zdiff.1', l + '.1',
        join_paths(get_option('mandir'), 'man1')
    )
endforeach

meson.add_install_script(install_link, 'zmore', 'zless', get_option('bindir'))
meson.add_install_script(
    install_link, 'zmore.1', 'zless.1',
    join_paths(get_option('mandir'), 'man1')
)
