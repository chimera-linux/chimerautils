uudecode_prog = executable(
    'uudecode',
    ['uudecode.c'],
    include_directories: [inc, cinc],
    link_with: [libcompat],
    install: true,
    install_dir: bindir,
)

install_symlink('b64decode', pointing_to: 'uudecode', install_dir: bindir)
