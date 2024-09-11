realpath = executable('realpath', ['realpath.cc'],
    include_directories: inc_main,
    install: true
)

if not chimera_realpath.enabled()
    install_symlink('readlink', pointing_to: 'realpath', install_dir: bindir)
endif
