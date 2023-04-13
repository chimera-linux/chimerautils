uuencode_prog = executable(
    'uuencode',
    ['uuencode.c'],
    include_directories: [inc, cinc],
    link_with: [libcompat],
    install: true,
    install_dir: bindir,
)

install_man('uuencode.1')
install_man('uuencode.format.5')

install_symlink('b64encode', pointing_to: 'uuencode', install_dir: bindir)
install_symlink('uudecode.1', pointing_to: 'uuencode.1', install_dir: man1dir)
install_symlink('b64encode.1', pointing_to: 'uuencode.1', install_dir: man1dir)
install_symlink('b64decode.1', pointing_to: 'b64encode.1', install_dir: man1dir)
install_symlink(
    'uudecode.5', pointing_to: 'uuencode.format.5', install_dir: man5dir
)
