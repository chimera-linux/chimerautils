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

if not zstd.found()
    extra_defs += [ '-DNO_ZSTD_SUPPORT' ]
endif

gzip_prog = executable(
    'gzip',
    [ 'gzip.c', ],
    include_directories : [ inc, cinc ],
    link_with : [ libcompat, libutil_static ],
    dependencies : [ zlib, bzip2, lzma, zstd, ],
    c_args : extra_defs,
    install : true,
)

install_man('gzip.1')

foreach s: [ 'gzexe', 'zdiff', 'zforce', 'zmore', 'znew' ]
    install_data(
        s,
        install_dir : bindir,
        install_mode : 'rwxr-xr-x',
    )
    install_man(s + '.1')
endforeach

foreach l: [ 'gunzip', 'gzcat', 'zcat' ]
    install_symlink(l, pointing_to: 'gzip', install_dir: bindir)
    install_symlink(l + '.1', pointing_to: 'gzip.1', install_dir: man1dir)
endforeach

foreach l: diff_links
    install_symlink(l, pointing_to: 'zdiff', install_dir: bindir)
    install_symlink(l + '.1', pointing_to: 'zdiff.1', install_dir: man1dir)
endforeach

install_symlink('zless', pointing_to: 'zmore', install_dir: bindir)
install_symlink('zless.1', pointing_to: 'zmore.1', install_dir: man1dir)
