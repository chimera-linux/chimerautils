realpath = executable('realpath', ['realpath.cc'],
    include_directories: inc_main,
    install: true
)

install_man('realpath.1')
install_man('readlink.1')

if not chimera_realpath.enabled()
    install_symlink('readlink', pointing_to: 'realpath', install_dir: bindir)
endif
