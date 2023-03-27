ctrlaltdel = executable('ctrlaltdel', ['ctrlaltdel.c'],
    include_directories: inc_main,
    install: true
)

install_symlink('reboot-mode', pointing_to: 'ctrlaltdel', install_dir: bindir)
